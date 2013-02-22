
#include "livetree.h"
#include "bin_utils.h"

using namespace swift;

Node::Node() : parent_(NULL), leftc_(NULL), rightc_(NULL), b_(bin_t::NONE), h_(Sha1Hash::ZERO), verified_(false)
{
}

void Node::SetParent(Node *parent)
{
    parent_ = parent; // TODOinline
}

Node *Node::GetParent()
{
    return parent_; // TODOinline
}


void Node::SetChildren(Node *leftc, Node *rightc)
{
    leftc_ = leftc;
    rightc_ = rightc;
}

Node *Node::GetLeft()
{
    return leftc_;
}

Node *Node::GetRight()
{
    return rightc_;
}


Sha1Hash &Node::GetHash()
{
    return h_;
}

void Node::SetHash(const Sha1Hash &hash)
{
    h_ = hash;
}

bin_t &Node::GetBin()
{
    return b_;
}

void Node::SetBin(bin_t &b)
{
    b_ = b;
}

bool Node::GetVerified()
{
    return verified_;
}

void Node::SetVerified(bool val)
{
    verified_ = val;
}


LiveHashTree::LiveHashTree(privkey_t privkey) : state_(LHT_STATE_SIGN_EMPTY), root_(NULL), addcursor_(NULL), peak_count_(0), size_(0), chunk_size_(SWIFT_DEFAULT_CHUNK_SIZE), privkey_(privkey), check_netwvshash_(false)
{
}

LiveHashTree::LiveHashTree(pubkey_t swarmid, bool check_netwvshash) : state_(LHT_STATE_VER_AWAIT_PEAK), root_(NULL), addcursor_(NULL), peak_count_(0), size_(0), chunk_size_(SWIFT_DEFAULT_CHUNK_SIZE), pubkey_(swarmid), check_netwvshash_(check_netwvshash)
{

}

LiveHashTree::~LiveHashTree ()
{
}

Node *LiveHashTree::GetRoot()
{
    return root_; // TODOinline
}

void LiveHashTree::SetRoot(Node *r)
{
    root_ = r;
}
void LiveHashTree::PurgeTree(Node *r)
{

}


// Live source specific
bin_t LiveHashTree::AddData(const char* data, size_t length)
{
    // Source adds new data

    if (addcursor_ == NULL)
	fprintf(stderr,"AddData: addcursor_ NULL\n");
    else
	fprintf(stderr,"AddData: addcursor_: %s\n", addcursor_->GetBin().str().c_str() );

    Sha1Hash hash(data,length);
    Node *next = CreateNext();
    next->SetHash(hash);

    fprintf(stderr,"AddData: set hash\n");

    // Calc new peaks
    size_ += length;
    peak_count_ = gen_peaks(size_in_chunks(),peak_bins_);

    state_ = LHT_STATE_SIGN_DATA;

    return next->GetBin();
}


Node *LiveHashTree::CreateNext()
{
    if (addcursor_ == NULL)
    {
	fprintf(stderr,"CreateNext: create root\n" );
	root_ = new Node();
	root_->SetBin(bin_t(0,0));
	addcursor_ = root_;
    }
    else if (addcursor_->GetBin().is_left())
    {
	// Left child, create sibling
	Node *newright = new Node();
	newright->SetBin(addcursor_->GetBin().sibling());

	fprintf(stderr,"CreateNext: create sibling %s\n", newright->GetBin().str().c_str() );

	Node *par = addcursor_->GetParent();
	if (par == NULL)
	{
	    // We was root, create new parent
	    par = new Node();
	    par->SetBin(bin_t(addcursor_->GetBin().layer()+1,0));
	    root_ = par;

	    fprintf(stderr,"CreateNext: create new root %s\n", root_->GetBin().str().c_str() );
	}
	par->SetChildren(addcursor_,newright);
	newright->SetParent(par);
	addcursor_->SetParent(par);
	addcursor_ = newright;
    }
    else
    {
	fprintf(stderr,"CreateNext: create tree\n");

	// We right child, need next
	Node *iter = addcursor_;
	while(true)
	{
	    iter = iter->GetParent();
	    fprintf(stderr,"CreateNext: create tree: check %s\n", iter->GetBin().str().c_str() );

	    if (iter == root_)
	    {
		// Need new root
		Node *newroot = new Node();
		newroot->SetBin(bin_t(iter->GetBin().layer()+1,0));

		fprintf(stderr,"CreateNext: create tree: new root %s\n", newroot->GetBin().str().c_str() );

		newroot->SetChildren(iter,NULL);
		root_ = newroot;
		iter->SetParent(newroot);
		iter = newroot;
	    }
	    if (iter->GetRight() == NULL) // not elsif
	    {
		// Create new subtree
		Node *newright = new Node();
		newright->SetBin(iter->GetBin().right());

		fprintf(stderr,"CreateNext: create tree: new right %s\n", newright->GetBin().str().c_str() );

		iter->SetChildren(iter->GetLeft(),newright);
		newright->SetParent(iter);

		// Need tree down to base layer
		int depth = iter->GetBin().layer()-1;

		iter = newright;
		Node *newleft = NULL;
		for (int i=0; i<depth; i++)
		{
		    newleft = new Node();
		    newleft->SetBin(iter->GetBin().left());

		    fprintf(stderr,"CreateNext: create tree: new left down %s\n", newleft->GetBin().str().c_str() );

		    iter->SetChildren(newleft,NULL);
		    newleft->SetParent(iter);

		    iter = newleft;
		}
		addcursor_ = newleft;
		break;
	    }
	    // if left, continue
	}
    }
    return addcursor_;

}


bool LiveHashTree::OfferSignedPeakHash(bin_t pos, const uint8_t *signedhash)
{
    // TODO check sig
    // TODO store sig
    Sha1Hash peakhash(false,(const char *)signedhash);

    fprintf(stderr,"OfferHash: offer is peak %s\n", pos.str().c_str() );
    peak_bins_[peak_count_++] = pos;

    sizec_ = peak_bins_[peak_count_].base_right().layer_offset();
    size_ = sizec_ * chunk_size_;

    if (state_ == LHT_STATE_VER_AWAIT_PEAK)
	state_ = LHT_STATE_VER_AWAIT_DATA;

    // Insert into tree
    (void)CreateAndVerifyNode(pos,peakhash,true);

    return true;
}


bool LiveHashTree::CreateAndVerifyNode(bin_t pos, const Sha1Hash &hash, bool verified)
{
    // This adds hashes on client side
    fprintf(stderr,"OfferHash: %s\n", pos.str().c_str() );

    if (state_ == LHT_STATE_VER_AWAIT_PEAK)
    {
	fprintf(stderr,"OfferHash: No peak yet, can't verify!" );
	return false;
    }

    // Find or create node
    Node *iter = root_;
    Node *parent = NULL;
    while (true)
    {
	if (iter == NULL)
	    fprintf(stderr,"OfferHash: iter NULL\n");
	else
	    fprintf(stderr,"OfferHash: iter %s\n", iter->GetBin().str().c_str() );

	if (iter == NULL)
	{
	    // Need to create some tree for it
	    if (parent == NULL)
	    {
		// No root
		root_ = new Node();
		root_->SetBin(pos);

		fprintf(stderr,"OfferHash: new root %s\n", root_->GetBin().str().c_str() );

		root_->SetHash(hash);
		return false;
	    }
	    else
	    {
		// Create left or right tree
		if (pos.toUInt() < parent->GetBin().toUInt())
		{
		    // Need node on left
		    Node *newleft = new Node();
		    newleft->SetBin(parent->GetBin().left());

		    fprintf(stderr,"OfferHash: create left %s\n", newleft->GetBin().str().c_str() );

		    newleft->SetParent(parent);

		    parent->SetChildren(newleft,parent->GetRight());
		    iter = newleft;
		}
		else
		{
		    // Need new node on right
		    Node *newright = new Node();
		    newright->SetBin(parent->GetBin().right());

		    fprintf(stderr,"OfferHash: create right %s\n", newright->GetBin().str().c_str() );

		    newright->SetParent(parent);

		    parent->SetChildren(parent->GetLeft(),newright);
		    iter = newright;
		}
	    }
	}
	else if (!iter->GetBin().contains(pos))
	{
	    // Offered pos not a child, error or create new root
	    Node *newroot = new Node();
	    newroot->SetBin(iter->GetBin().parent());

	    fprintf(stderr,"OfferHash: new root no cover %s\n", newroot->GetBin().str().c_str() );

	    if (pos.layer_offset() < iter->GetBin().layer_offset())
		newroot->SetChildren(NULL,iter);
	    else
		newroot->SetChildren(iter,NULL);
	    root_ = newroot;
	    iter->SetParent(newroot);
	    iter = newroot;
	}

	if (pos.toUInt() == iter->GetBin().toUInt())
	{
	    // Found it
	    fprintf(stderr,"OfferHash: found node %s\n", iter->GetBin().str().c_str() );
	    break;
	}
	else if (pos.toUInt() < iter->GetBin().toUInt())
	{
	    fprintf(stderr,"OfferHash: go left\n" );

	    parent = iter;
	    iter = iter->GetLeft();
	}
	else
	{
	    fprintf(stderr,"OfferHash: go right\n" );

	    parent = iter;
	    iter = iter->GetRight();
	}
    }


    // TODO Test if known
    if (iter == NULL)
    {
	fprintf(stderr,"OfferHash: internal error, couldn't find or create node\n" );
	return false;
    }

    // From MmapHashTree

    //NETWVSHASH
    if (!check_netwvshash_)
    	return true;

    bin_t peak = peak_for(pos);
    if (peak.is_none())
        return false;
    if (peak==pos)
    {
	if (verified)
	{
	    iter->SetHash(hash);
	    iter->SetVerified(verified);
	}
        return hash == iter->GetHash();
    }
    if (!ack_out_.is_empty(pos.parent()))
        return hash == iter->GetHash(); // have this hash already, even accptd data
    // LESSHASH
    // Arno: if we already verified this hash against the root, don't replace
    if (iter->GetVerified())
        return hash == iter->GetHash();

    iter->SetHash(hash);
    if (!pos.is_base())
        return false; // who cares?
    bin_t p = pos;
    Node *piter = iter;
    Sha1Hash uphash = hash;

    fprintf(stderr,"OfferHash: verifying %s\n", pos.str().c_str() );
    // Walk to the nearest proven hash
    while ( p!=peak && ack_out_.is_empty(p) && !piter->GetVerified() ) {
        piter->SetHash(uphash);
        p = p.parent();
        piter = piter->GetParent();

        // Arno: Prevent poisoning the tree with bad values:
        // Left hand hashes should never be zero, and right
        // hand hash is only zero for the last packet, i.e.,
        // layer 0. Higher layers will never have 0 hashes
        // as SHA1(zero+zero) != zero (but b80de5...)
        //

        fprintf(stderr,"OfferHash: squirrel %s %p %p\n", p.str().c_str(), piter->GetLeft(), piter->GetRight() );

        if (piter->GetLeft() == NULL || piter->GetRight() == NULL)
            return false; // tree still incomplete

        fprintf(stderr,"OfferHash: hashsquirrel %s %s %s\n", p.str().c_str(), piter->GetLeft()->GetHash().hex().c_str(), piter->GetRight()->GetHash().hex().c_str() );

        if (piter->GetLeft()->GetHash() == Sha1Hash::ZERO || piter->GetRight()->GetHash() == Sha1Hash::ZERO)
            break;
        uphash = Sha1Hash(piter->GetLeft()->GetHash(),piter->GetRight()->GetHash());
    }

    fprintf(stderr,"OfferHash: verify computed against %s %s\n", piter->GetBin().str().c_str(), piter->GetHash().hex().c_str() );

    bool success = (uphash==piter->GetHash());
    // LESSHASH
    if (success) {
        // Arno: The hash checks out. Mark all hashes on the uncle path as
        // being verified, so we don't have to go higher than them on a next
        // check.
        p = pos;
        piter = iter;
        piter->SetVerified(true);
        while (p.layer() != peak.layer()) {
            p = p.parent().sibling();
            if (piter->GetParent()->GetBin().is_left())
        	piter = piter->GetParent()->GetRight();
            else
        	piter = piter->GetParent()->GetLeft();
            piter->SetVerified(true);
        }
        // Also mark hashes on direct path to root as verified. Doesn't decrease
        // #checks, but does increase the number of verified hashes faster.
        p = pos;
        piter = iter;
        while (p != peak) {
            p = p.parent();
            piter = piter->GetParent();
            piter->SetVerified(true);
        }
    }

    return success;
}



// HashTree interface

bool LiveHashTree::OfferHash(bin_t pos, const Sha1Hash& hash)
{
    return CreateAndVerifyNode(pos,hash,false);

}


bool LiveHashTree::OfferData(bin_t pos, const char* data, size_t length)
{
    if (state_ == LHT_STATE_VER_AWAIT_PEAK)
        return false;
    if (!pos.is_base())
        return false;
    if (length<chunk_size_ && pos!=bin_t(0,sizec_-1))
        return false;
    if (ack_out_.is_filled(pos))
        return true; // to set data_in_
    bin_t peak = peak_for(pos);
    if (peak.is_none())
        return false;

    Sha1Hash data_hash(data,length);
    if (!OfferHash(pos, data_hash)) {
        //printf("invalid hash for %s: %s\n",pos.str(bin_name_buf),data_hash.hex().c_str()); // paranoid
        //fprintf(stderr,"INVALID HASH FOR %lli layer %d\n", pos.toUInt(), pos.layer() );
        // Ric: TODO it's not necessarily a bug.. it happens if a pkt was lost!
        dprintf("%s hashtree check failed (bug TODO) %s\n",tintstr(),pos.str().c_str());
        return false;
    }

    //printf("g %lli %s\n",(uint64_t)pos,hash.hex().c_str());
    ack_out_.set(pos);
#ifdef TODO
    // Arno,2011-10-03: appease g++
    if (storage_->Write(data,length,pos.base_offset()*chunk_size_) < 0)
        print_error("pwrite failed");
    complete_ += length;
    completec_++;
    if (pos.base_offset()==sizec_-1) {
        size_ = ((sizec_-1)*chunk_size_) + length;
        if (storage_->GetReservedSize()!=size_)
            storage_->ResizeReserved(size_);
    }
#endif
    return true;

}


int  LiveHashTree::peak_count() const
{
    return peak_count_; // TODOinline
}

bin_t LiveHashTree::peak(int i) const
{
    return peak_bins_[i]; // TODOinline
}

const Sha1Hash& LiveHashTree::peak_hash(int i) const
{
    return hash(peak(i)); // TODOinline
}

bin_t LiveHashTree::peak_for(bin_t pos) const
{
    for (int i=0; i<peak_count_; i++)
    {
	if (peak_bins_[i].contains(pos))
	    return peak_bins_[i];
    }
    return bin_t::NONE;
}

const Sha1Hash& LiveHashTree::hash(bin_t pos) const
{
    // This API may not be fastest with dynamic tree.
    Node *iter = root_;
    while (true)
    {
	if (iter == NULL)
	    return Sha1Hash::ZERO;
	else if (pos.toUInt() == iter->GetBin().toUInt())
	    return iter->GetHash();
	else if (pos.toUInt() < iter->GetBin().toUInt())
	    iter = iter->GetLeft();
	else
	    iter = iter->GetRight();
    }
}

const Sha1Hash& LiveHashTree::root_hash() const
{
    if (root_ == NULL)
	return Sha1Hash::ZERO;
    else
	return root_->GetHash();
}

uint64_t LiveHashTree::size() const
{
    return size_;
}

uint64_t LiveHashTree::size_in_chunks() const
{
    return size_/chunk_size_; // TODOinline
}


uint64_t LiveHashTree::complete() const
{
    return 0;
}

uint64_t LiveHashTree::chunks_complete() const
{
    return 0;
}

uint64_t LiveHashTree::seq_complete(int64_t offset)
{
    return 0;
}
bool LiveHashTree::is_complete()
{
    return false;
}
binmap_t *LiveHashTree::ack_out ()
{
    return &ack_out_;
}

uint32_t LiveHashTree::chunk_size()
{
    return chunk_size_; // TODOinline
}


bool LiveHashTree::get_check_netwvshash(void)
{
    return false;
}
Storage *LiveHashTree::get_storage()
{
    return NULL;
}
void LiveHashTree::set_size(uint64_t)
{
}
int LiveHashTree::TESTGetFD()
{
    return 481;
}


void LiveHashTree::sane_tree()
{
    if (root_ == NULL)
	return;
    else
    {
	sane_node(root_,NULL);
    }
}


void LiveHashTree::sane_node(Node *n, Node *parent)
{
    //fprintf(stderr,"Sane: %s\n", n->GetBin().str().c_str() );
    assert(n->GetParent() == parent);
    if (n->GetLeft() != NULL)
    {
	assert(n->GetLeft()->GetParent() == n);
	sane_node(n->GetLeft(),n);
    }
    if (n->GetRight() != NULL)
    {
	assert(n->GetRight()->GetParent() == n);
	sane_node(n->GetRight(),n);
    }
}

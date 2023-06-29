/**
 * @file imgtree.cpp
 * @description implementation of ImgTree class used for storing image data
 *              CPSC 221 2023S PA3
 *
 *              SUBMIT THIS FILE WITH YOUR MODIFICATIONS
 */

#include "imgtree.h"
#include <limits>

/* ImgTree constructor */
ImgTree::ImgTree(const PNG& imIn){ 
	// complete your implementation below
	Stats stat(imIn);

	// build the root node and tree
	width = imIn.width();
	height = imIn.height();
	std::pair<unsigned int, unsigned int> ul = {0, 0};
	std::pair<unsigned int, unsigned int> lr = {width-1, height-1};
	root = BuildTree(stat, ul, lr);
}

/* BuildTree helper for ImgTree constructor */
Node* ImgTree::BuildTree(Stats& s, pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr) {
	// replace the stub below with your own implementation
	if (ul == lr) return new Node(ul, lr, s.GetAvg(ul,lr));

	Node* curr = new Node(ul, lr, s.GetAvg(ul,lr));

	// we need four pairs to make two splitted rectangles
	// the left rectangle which consists of left_ul and left_lr
	// the right rectangle which consists of right_ul and right_lr
	pair<pair<pair<unsigned int, unsigned int>,  // first.first
		pair<unsigned int, unsigned int>>, 		// first.second
			pair<pair<unsigned int, unsigned int>, //second.first
				pair<unsigned int, unsigned int>>> split //second.second
					= SplitPos(s, ul, lr); 

	pair<unsigned int, unsigned int> lt_ul = split.first.first;
	pair<unsigned int, unsigned int> lt_lr = split.first.second;
	pair<unsigned int, unsigned int> rb_ul = split.second.first;
	pair<unsigned int, unsigned int> rb_lr = split.second.second;

    curr->LT = BuildTree(s, lt_ul, lt_lr);
    curr->RB = BuildTree(s, rb_ul, rb_lr);

	return curr;
}

/* Render your ImgTree into a png */
PNG ImgTree::Render() const{
	// replace the stub below with your own implementation
	PNG image(width, height);
    RenderHelper(root, image);
    return image;
}

/* Prune function modifies tree by cutting off
 * subtrees whose leaves are all within tol of 
 * the average pixel value contained in the root
 * of the subtree
 */
void ImgTree::Prune(double tol){
	// complete your implementation below
	PruneHelper(root, tol);
}

/* Modifies node contents so that the rendered tree
 * will appear to be flipped horizontally across
 * a vertical axis
 */
void ImgTree::FlipHorizontal() {
	// complete your implementation below
	FlipHorizontalHelper(root, 0); 
}

/* helper function for destructor and op= */
/* frees dynamic memory associated w the ImgTree */
void ImgTree::Clear() {
	// complete your implementation below
	ClearHelper(root); // Start the recursive clearing from the root
    root = nullptr; // Reset the root pointer
    
}

/* helper function for copy constructor and op= */
void ImgTree::Copy(const ImgTree& orig){
	// complete your implementation below
	root = CopyHelper(orig.root);
    width = orig.width;
    height = orig.height;
    
}

/**
 * Add your private ImgTree function implementations below
 */

/* Recursive helper function for Prune */
vector<HSLAPixel> ImgTree::PruneHelper(Node* node, double tol) {
    vector<HSLAPixel> leaves;

    // if empty tree
    if (node == nullptr) {
        return leaves;
    }

    // if arrived at leaf node
    if (node->LT == nullptr && node->RB == nullptr) {
        leaves.push_back(node->avg);
        return leaves;
    }

    // assume by now at non-leaf node, combine the leaves of the left and right branches
    vector<HSLAPixel> left_leaves = PruneHelper(node->LT, tol);
    vector<HSLAPixel> right_leaves = PruneHelper(node->RB, tol);

    // if one of the branches was a non-compliant node
    if (left_leaves.empty() || right_leaves.empty()) {
        leaves.clear();
        return leaves;
    }

    leaves.insert(leaves.end(), left_leaves.begin(), left_leaves.end());
    leaves.insert(leaves.end(), right_leaves.begin(), right_leaves.end());

    // check leaves vs current avg
    for (const HSLAPixel& px : leaves) {
        if (node->avg.distanceTo(px) > tol) {
            return leaves;
        }
    }

    // assume by now, the current avg is within tolerance of all leaves under it
    ClearHelper(node->LT);
    ClearHelper(node->RB);
    node->LT = nullptr;
    node->RB = nullptr;

    return leaves;
}

pair<pair<pair
	<unsigned int, unsigned int>, 
		pair<unsigned int, unsigned int>>, 
			pair<pair<unsigned int, unsigned int>, 
				pair<unsigned int, unsigned int>>> ImgTree::SplitPos(Stats& s, pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr) {

	pair<pair<pair
		<pair<unsigned int, unsigned int>, //first.first.first
			pair<unsigned int, unsigned int>>, // first.first.second
				pair<pair<unsigned int, unsigned int>, //first.second.first
					pair<unsigned int, unsigned int>>>, //first.second.second
						double> minVariability; // second (double is for entropy check)

	minVariability.second = std::numeric_limits<double>::max(); // sets minimum variability to the highest num
	// check if the shape is square/wide or tall
	if (lr.first-ul.first >= lr.second-ul.second) { // square or wide
		// iterate vertically
		// set out the first split

			/*	split mechanics:
				+---+----------+
				|lul|rul 	   |
				|   | 		   |
				|   | 		   |
				|llr| 		rlr|
				+---+----------+
			*/
		for (unsigned int index = ul.first; index < lr.first; index++) { // not <= so that we can use index+1 for splitting
			// create a vertical split
			std::pair<unsigned int, unsigned int> left_ul = {ul.first, ul.second};
			std::pair<unsigned int, unsigned int> left_lr = {index, lr.second};

			std::pair<unsigned int, unsigned int> right_ul = {index+1, ul.second};
			std::pair<unsigned int, unsigned int> right_lr = {lr.first, lr.second};

			// compute weighted entropy average	
			double entropy_left = s.Entropy(left_ul, left_lr);
			long area_left = s.RectArea(left_ul,left_lr);

			double entropy_right = s.Entropy(right_ul, right_lr);
			long area_right = s.RectArea(right_ul,right_lr);

			double weighted_avg = ((entropy_left*area_left) + (entropy_right*area_right))/(area_left+area_right);

			if (weighted_avg < minVariability.second) {
				minVariability.first.first.first = left_ul;     // Update the first coordinate pair with the split position
    			minVariability.first.first.second = left_lr;    // Update the second coordinate pair with the split position

				minVariability.first.second.first = right_ul;     // Update the first coordinate pair with the split position
    			minVariability.first.second.second = right_lr;    // Update the second coordinate pair with the split position
    			minVariability.second = weighted_avg;  
			}
		}
	} else { // tall
			// iterate horizontally
			// set out the first split

			/*	split mechanics:
				+--------------+
				|uul        ulr|
				+--------------+ 
				|dul    	   |
				| 			dlr|
				+--------------+
			*/
		for (unsigned int index = ul.second; index < lr.second; index++) { // not <= so that we can use index+1 for splitting
			// create a horizontal split
			std::pair<unsigned int, unsigned int> up_ul = {ul.first, ul.second};
			std::pair<unsigned int, unsigned int> up_lr = {lr.first, index};

			std::pair<unsigned int, unsigned int> down_ul = {ul.first, index+1};
			std::pair<unsigned int, unsigned int> down_lr = {lr.first, lr.second};

			// compute weighted entropy average	
			double entropy_up = s.Entropy(up_ul, up_lr);
			long area_up = s.RectArea(up_ul,up_lr);

			double entropy_down = s.Entropy(down_ul, down_lr);
			long area_down = s.RectArea(down_ul,down_lr);

			double weighted_avg = ((entropy_up*area_up) + (entropy_down*area_down))/(area_up+area_down);

			if (weighted_avg < minVariability.second) {
				minVariability.first.first.first = up_ul;     // Update the first coordinate pair with the split position
    			minVariability.first.first.second = up_lr;    // Update the second coordinate pair with the split position

				minVariability.first.second.first = down_ul;     // Update the first coordinate pair with the split position
    			minVariability.first.second.second = down_lr;    // Update the second coordinate pair with the split position
    			minVariability.second = weighted_avg;  
			}
		}
	}
	return minVariability.first; // returns the four pairs only.
}

void ImgTree::RenderHelper(Node* node, PNG& image) const {
    if (node == nullptr) return;

    if (node->LT == nullptr && node->RB == nullptr) {
        for (unsigned int y = node->upLeft.second; y <= node->lowRight.second; y++) {
            for (unsigned int x = node->upLeft.first; x <= node->lowRight.first; x++) {
                HSLAPixel* pixel = image.getPixel(x, y);
                *pixel = node->avg;
            }
        }
    } else {
        RenderHelper(node->LT, image);
        RenderHelper(node->RB, image);
    }
}

void ImgTree::ClearHelper(Node* node) {
    if (node == nullptr) return; // Base case: empty subtree

    // Recursively clear the left and right subtrees
    ClearHelper(node->LT);
    ClearHelper(node->RB);

    // Deallocate the current node
    delete node;
}

Node* ImgTree::CopyHelper(const Node* node) {
    if (node == nullptr) return nullptr; // Base case: empty subtree

    // Create a new node with the same properties as the original node
    Node* copyNode = new Node(node->upLeft, node->lowRight, node->avg);

    // Recursively copy the left and right subtrees
    copyNode->LT = CopyHelper(node->LT);
    copyNode->RB = CopyHelper(node->RB);

    return copyNode;
}

void ImgTree::FlipHorizontalHelper(Node* node, int offset) {
    if (node == nullptr) return;

	// initialize the current offset
	int LT_offset = 0; int RB_offset = 0;

	// offset is to pass the moving x values of each coordinate
	// if it's an LT node, offset will probably be negative to move your x value to the left
	// and positive if it's an RB node to move it to the right
	node->upLeft.first += offset;
	node->lowRight.first += offset;

	// if the current node is a wide rectangle or square, we update the offset and swap values
	if (node->lowRight.first-node->upLeft.first >= node->lowRight.second-node->upLeft.second &&
		node->LT != nullptr && node->RB != nullptr) {

		// first to keep track before changing anything, get the new offsets
		LT_offset = node->LT->upLeft.first - node->RB->upLeft.first;
		RB_offset = node->RB->lowRight.first - node->LT->lowRight.first;

		// swap their structural position	
		Node* temp = node->LT;
		node->LT = node->RB;
		node->RB = temp;
	}
	// recurse
	FlipHorizontalHelper(node->LT, offset + LT_offset);
	FlipHorizontalHelper(node->RB, offset + RB_offset);
}

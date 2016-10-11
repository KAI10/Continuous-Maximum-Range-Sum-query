/*
 * maxrs_utilities.h
 *
 * Created by: Ashik <ashik@KAI10>
 * Created on: 2016-10-09
 */

using namespace std;
#include "Area.h"
#include "Object.h"
#include "Window.h"
#include "Rectangle.h"
#include "IntervalTree.h"

#include <vector>
#include <set>

void restrict_precision(double &val)
{
    val = roundf(val * 100000) / 100000;
}

void restrict_precision(vector<Object> &objects)
{
    for(int i=0; i<objects.size(); i++){
        objects[i].x = roundf(objects[i].x * 100000) / 100000;
        objects[i].y = roundf(objects[i].y * 100000) / 100000;
    }
}

/*
  Maximum Rectangle Enclosing Algorithm using Interval Tree
  Implementation based on paper: S.C. Nandy and B.B. Bhattacharya. "A
  Unified Algorithm for Finding Maximum and Minimum Object Enclosing
  Rectangles and Cuboids."  Computer Math Application. Vol
  29. No.8. 1995

  Author: Chunxiao Diao
  Rewritten in Java: Panitan Wongse-ammat
  Rewritten in C++: Kazi Ashik Islam
*/

//public class Meit
//{
// global variables
// used in incToNode functions to determine whether an interval
// overlapping/intersecting the left or right of current interval has
// been found
bool left_found = false;
bool right_found = false;

// 1 is the left one, 2 is the right one
// used to help determine whether to merge two adjacent intervals in
// decToNode leftInoput 1 and 2 are the intervals to the left and to the right
// of the left point of current interval respectively. e.g current
// interval -> [2,4] left_intersect1 = [x,2] left_intersect2 = [2,y]
// x < 2 and 2<y<=4 same applies to right_intersect
IntervalTree *left_intersect1 = NULL;
IntervalTree *left_intersect2 = NULL;
IntervalTree *right_intersect1 = NULL;
IntervalTree *right_intersect2 = NULL;

IntervalTree *interval_tree_root = NULL;

// build a balanced interval tree
// use the average of two median points as root; attach left nodes(nodes
// to the left of right medians) as left subtrees and right nodes
// (nodes to the right of left median) as right subtees
IntervalTree* buildIntervalTree(int st, int ed, vector<double> listOfPoints, IntervalTree *root)
{
    if (st == ed){
        IntervalTree *leaf_node = new IntervalTree(listOfPoints[st], root);
        leaf_node->window = new Window(listOfPoints[st], // l
                                      listOfPoints[st], // r
                                      (double)(-5), // h
                                      (double)(0)); // score
        return leaf_node;
    }

    int mid = (st + ed) / 2;

    /*
    System.out.println("st: " + st + " mid: " + mid + " ed: " + ed);
    System.out.println("listOfPoint: [mid]:" + listOfPoints.get(mid)
                       + " [mid+1]:" + listOfPoints.get(mid+1) +
                       " div2:" + (listOfPoints.get(mid)+ listOfPoints.get(mid+1))/2);
    */

    IntervalTree *new_node = new IntervalTree((listOfPoints[mid] + listOfPoints[mid+1])/2, root);

    new_node->left_child = buildIntervalTree(st, mid, listOfPoints, new_node);
    new_node->right_child = buildIntervalTree(mid+1, ed, listOfPoints, new_node);
    return new_node;
}

void preOrderTraverse(IntervalTree *root)
{
    if(root == NULL) return;

    //System.out.println(root);
    root->display();
    preOrderTraverse(root->left_child);
    preOrderTraverse(root->right_child);
}

// propagate Excess of a father node to its two child nodes.
void propagateExcess(IntervalTree *root, double h)
{
    ///puts("inside propagateExcess");
    if(root->excess != 0)
    {
        if(root->left_child != NULL)
        {
            root->left_child->excess += root->excess;
            root->left_child->maxscore += root->excess;
            if(root->left_child->window != NULL)
            {
                root->left_child->window->score += root->excess;
                root->left_child->window->h = h;
            }
        }
        if(root->right_child != NULL)
        {
            root->right_child->excess += root->excess;
            root->right_child->maxscore += root->excess;
            if(root->right_child->window != NULL)
            {
                root->right_child->window->score += root->excess;
                root->right_child->window->h = h;
            }
        }
    }
    root->excess = 0;
}


IntervalTree* findNodeV(IntervalTree *root, double l, double r, double h)
{
    ///puts("inside findNodeV");
    propagateExcess(root, h);
    ///GETTING SEGMENTATION FAULT HERE
    ///puts("After propagateExcess");

    if(root->discriminant < l) return findNodeV(root->right_child, l, r, h);
    else if(root->discriminant > r) return findNodeV(root->left_child, l, r, h);
    return root;
}

IntervalTree* findLeafNode(IntervalTree *root, double v, double h)
{
    propagateExcess(root, h);
    if(root->discriminant == v) return root;
    if(root->discriminant > v) return findLeafNode(root->left_child, v, h);
    if(root->discriminant < v) return findLeafNode(root->right_child, v, h);
    return NULL; // suppress warning
}

// insert window [a,b] to the first node with discriminant larger than a
// and smaller than b
IntervalTree* insertWindow(Window *window, IntervalTree *root)
{
    if(window == NULL) return NULL;
    if(root->discriminant <= window->r && root->discriminant >= window->l)
    {
        root->window = window;
        return root;
    }
    else if(root->discriminant < window->l) return insertWindow(window, root->right_child);
    return insertWindow(window, root->left_child);
}

// when the bottom of a rectangle is processed, we add ([l,r]) it
// into the interval tree
IntervalTree* incToNodeV(double l, double r, double h, double weight, IntervalTree *root)
{
    if(root->window != NULL)
    {
        // a window [a,b] in the tree contains interval [l,r]
        // in this case , we break it into three new windows [a,l] [l,r] and [r,b]
        if((!left_found) && (!right_found) && (root->window->l <= l) && (root->window->r >= r))
        {
            Window *left_window = NULL;
            Window *right_window = NULL;
            // [l, r] -> [root.window.l, l] [l,r] [r, root.window.r]
            if(root->window->l < l) left_window = new Window(root->window->l, l, h, root->window->score);
            if(root->window->r > r) right_window = new Window(r, root->window->r, h, root->window->score);
            Window *mid_window = new Window(l, r, h, root->window->score + weight);

            insertWindow(left_window, root);
            insertWindow(right_window, root);
            insertWindow(mid_window, root);
            left_found = true;
            right_found = true;
        }
        else if ((root->window->l < l) && (root->window->r > l) && (!left_found))
        {
            // a window [a,b] overlap with the left part of interval
            // breaks the window into two windows [a,l] [l,b]
            // or [root.window.l, l] [l, root.window.r]
            Window *mid_window = new Window(l, root->window->r, h, root->window->score+weight);
            Window *left_window = new Window(root->window->l, l, h, root->window->score);
            insertWindow(left_window, root);
            insertWindow(mid_window, root);
            left_found = true;
        }
        else if((root->window->l < r) && (root->window->r > r) && (!right_found))
        {
            // a window [a,b] overlap with the right part of interval
            // breaks the window into two windows [a,r] [r,b]
            // [root.window.l, r] [r, root.window. r]
            Window *right_window = new Window(r, root->window->r, h, root->window->score);
            Window *mid_window = new Window(root->window->l, r, h, root->window->score+weight);
            insertWindow(right_window, root);
            insertWindow(mid_window, root);
            right_found = true;
        }
        else if((root->window->l >= l) && (root->window->r <= r) &&
                (! ((left_found && (root->window->l == l)) ||
                    (right_found && (root->window->r == r)))))
        {
            root->window->score += weight;
            root->window->h = h;
        }

        if(root->discriminant > r) return incToNodeV(l, r, h, weight, root->left_child);
        else if(root->discriminant < l) return incToNodeV(l, r, h, weight, root->right_child);
        return root;
    }
    return NULL; // suppress warnings
}

IntervalTree* incToNodeL(double l, double r, double h, double weight, IntervalTree *root)
{
    if(root->window != NULL){
        // left overlapping
        // a window [a,b] overlap with the left part of interval
        // breaks the window into two windows [a,l] [l,b]
        if((root->window->l < l) && (root->window->r > l) &&
                (!left_found))
        {
            Window *mid_window = new Window(l, root->window->r, h, root->window->score+weight);
            Window *left_window = new Window(root->window->l, l, h, root->window->score);
            insertWindow(left_window, root);
            insertWindow(mid_window, root);
            left_found = true;
        }
        else if ((root->window->l >= l) && (root->window->r <= r) &&
                 ((! ((left_found && (root->window->l == l)) ||
                      (right_found && root->window->r == r))) ||
                  (root->window->r == root->window->l)))
        {
            // interval contains window
            root->window->score += weight;
            root->window->h = h;
        }
    }


    if(root->discriminant == l) return root;
    else if(root->discriminant < l) return incToNodeL(l, r, h, weight, root->right_child);
    else if (root->discriminant > l)
    {
        // right subtree must be contained in the interval
        // change the root's excess
        root->right_child->excess += weight;
        root->right_child->maxscore += weight;
        if(root->right_child->window != NULL)
        {
            root->right_child->window->score += weight;
            root->right_child->window->h = h;
        }
        return incToNodeL(l, r, h, weight, root->left_child);
    }
    return NULL;
}

IntervalTree* incToNodeR(double l, double r, double h, double weight, IntervalTree *root)
{
    if(root->window != NULL)
    {
        if((root->window->r > r) && (root->window->l < r) && (!right_found))
        {
            // a window [a,b] overlap with the right part of interval
            // breaks the window into two windows [a,r] [r,b]
            Window *right_window = new Window(r, root->window->r, h, root->window->score);
            Window *mid_window = new Window(root->window->l, r, h, root->window->score+weight);
            insertWindow(right_window, root);
            insertWindow(mid_window, root);
            right_found = true;
        }
        else if ((root->window->l >= l) && (root->window->r <= r) &&
                 ((!((left_found && (root->window->l == l)) ||
                     (right_found && (root->window->r == r)))) ||
                  (root->window->r == root->window->l)))
        {
            root->window->score += weight;
            root->window->h = h;
        }
    }

    if(root->discriminant == r) return root;
    else if(root->discriminant > r) return incToNodeR(l, r, h, weight, root->left_child);
    else if(root->discriminant < r)
    {
        // left subtree must be contained in the interval
        // change the root's excess
        root->left_child->excess += weight;
        root->left_child->maxscore += weight;
        if(root->left_child->window != NULL)
        {
            root->left_child->window->score += weight;
            root->left_child->window->h = h;
        }
        return incToNodeR(l, r, h, weight, root->right_child);
    }
    return NULL; // suppress warnings
}

// backward path. compare the maximum of a node's two children and the
// score of the window on current node
// chose the largest one as the local maximum
Window* updateToNode(IntervalTree *cur, IntervalTree *end_node)
{
    if((cur->window != NULL) &&
            ((cur->left_child == NULL) || (cur->window->score > cur->left_child->maxscore)) &&
            ((cur->right_child == NULL) || (cur->window->score > cur->right_child->maxscore)))
    {
        cur->maxscore = cur->window->score;
        cur->target = cur->window;
    }
    else if((cur->left_child != NULL) &&
            ((cur->right_child == NULL) ||
             cur->left_child->maxscore > cur->right_child->maxscore))
    {
        cur->maxscore = cur->left_child->maxscore;
        cur->target = cur->left_child->target;
    }
    else if(cur->right_child != NULL)
    {
        cur->maxscore = cur->right_child->maxscore;
        cur->target = cur->right_child->target;
    }

    if(cur == end_node) return cur->target;
    return updateToNode(cur->father, end_node);
}

// processing the bottom of a rectangle
// propogate excess first
// then go through the tree to find overlapping or containing windows
// the bottom of the rectangle is namecoded as "the interval"
// the intervals or windows in the interval tree are namecoded "window"
void incIntervalTree(double h, double l, double r, double weight, IntervalTree *root)
{
    left_found = false;
    right_found = false;

    IntervalTree *node_v = findNodeV(root, l, r, h);
    IntervalTree *node_l = findLeafNode(root, l, h);
    IntervalTree *node_r = findLeafNode(root, r, h);

    /*
    System.out.println("node_v: " + node_v);
    System.out.println("node_l: " + node_l);
    System.out.println("node_r: " + node_r);
    */

    incToNodeV(l, r, h, weight, root);

    incToNodeL(l, r, h, weight, node_v->left_child);

    incToNodeR(l, r, h, weight, node_v->right_child);

    updateToNode(node_l, node_v);
    updateToNode(node_r, node_v);
    updateToNode(node_v, root);
}

// processing the top of a rectangle case by case
// leaf nodes represent a point and have windows [a,a]
void decToNode(double l, double r, double h, double weight, IntervalTree *root, char flag)
{
    if(root->window != NULL)
    {
        if(root->window->l < root->window->r)
        {
            // since we do not break windows on leaf nodes, we need to
            // check whether the current node is a leaf node first
            // try to find the adjacent pairs that intersect on l and r
            if(root->window->l == l) left_intersect2 = root;
            if(root->window->r == l) left_intersect1 = root;
            if(root->window->l == r) right_intersect2 = root;
            if(root->window->r == r) right_intersect1 = root;
        }
        // if the interval contains the window, change the score of the window
        if((root->window->l >= l) && (root->window->r <= r))
        {
            root->window->score -= weight;
            root->window->h = h;
        }

        if((left_intersect1 != NULL) && (left_intersect2 != NULL))
        {
            // two adjacent windows that intersect on l are found
            // delete the current window(if the weight difference is
            // equal to the current weight of the interval)
            // merge two windows into the one that is closer to the root
            if((left_intersect1->window->score == left_intersect2->window->score))
            {
                Window *new_window = new Window(left_intersect1->window->l,
                                               left_intersect2->window->r,
                                               left_intersect2->window->h,
                                               left_intersect2->window->score);
                left_intersect1->window = NULL;
                left_intersect2->window = NULL;
                if(new_window->r == r) right_intersect1 = insertWindow(new_window, interval_tree_root);
            }
            left_intersect1 = NULL;
            left_intersect2 = NULL;
        }

        if((right_intersect1 != NULL) && (right_intersect2 != NULL))
        {
            // two adjacent windows that intersect on r are found
            // delete the current window, merge both windows into the
            // adjacent one which is closer to the root
            if(right_intersect1->window->score == right_intersect2->window->score)
            {
                Window *new_window = new Window(right_intersect1->window->l,
                                               right_intersect2->window->r,
                                               right_intersect1->window->h,
                                               right_intersect1->window->score);
                right_intersect1->window = NULL;
                right_intersect2->window = NULL;
                // if the left window happens to intersect with point
                // r, the new merged window needs to be marked
                if(new_window->l == l) left_intersect2 = insertWindow(new_window, interval_tree_root);
                root->window = NULL;
            }
            right_intersect1 = NULL;
            right_intersect2 = NULL;
        }
    }

    if(flag == 'v')
    {
        if((root->discriminant > l) && (root->discriminant < r)) return;
        else if(r < root->discriminant) decToNode(l, r, h, weight, root->left_child, flag);
        else if(l > root->discriminant) decToNode(l, r, h, weight, root->right_child, flag);
    }
    else if(flag == 'l')
    {
        if(root->discriminant == l) return;
        else if(l < root->discriminant)
        {
            root->right_child->excess -= weight;
            root->right_child->maxscore -= weight;
            if(root->right_child->window != NULL)
            {
                root->right_child->window->score -= weight;
                root->right_child->window->h = h;
            }
            decToNode(l, r, h, weight, root->left_child, flag);
        }
        else if(l > root->discriminant) decToNode(l, r, h, weight, root->right_child, flag);
    }
    else if(flag == 'r')
    {
        if(root->discriminant == r) return;
        else if(r < root->discriminant) decToNode(l, r, h, weight, root->left_child, flag);
        else if(r > root->discriminant)
        {
            root->left_child->excess -= weight;
            root->left_child->maxscore -= weight;
            if(root->left_child->window != NULL)
            {
                root->left_child->window->score -= weight;
                root->left_child->window->h = h;
            }
            decToNode(l, r, h, weight, root->right_child, flag);
        }
    }
}

// processing the top of a rectangle
// propogate excess first
// then traverse the interval tree to merge or change affected windows
void decIntervalTree(double h, double l, double r, double weight, IntervalTree *root)
{

    left_intersect1 = NULL;
    left_intersect2 = NULL;
    right_intersect1 = NULL;
    right_intersect2 = NULL;

    IntervalTree *node_v = findNodeV(root, l, r, h);

    IntervalTree *node_l = findLeafNode(root, l, h);
    IntervalTree *node_r = findLeafNode(root, r, h);

    /*
    System.out.println("node_v: " + node_v);
    System.out.println("node_l: " + node_l);
    System.out.println("node_r: " + node_r);
    */

    decToNode(l, r, h, weight, root, 'v');
    decToNode(l, r, h, weight, node_v->left_child, 'l');
    decToNode(l, r, h, weight, node_v->right_child, 'r');

    updateToNode(node_l, node_v);
    updateToNode(node_r, node_v);
    updateToNode(node_v, root);

}


Window* maxEnclosing(vector<Rectangle> &aListOfRectangles, Area coverage, IntervalTree *root)
{
    // optimal answer
    Window *optimalWindow = new Window(0, 0, 0, 0);
    // top index is the index of the next rectangle whose bottom should
    // be added into interval tree (Note: top index is for bottom edge
    // of a rectangle) it can be interpreted as the top lane of a sweep
    // lane algorithm. the active rectangles are between top index and
    // bot index
    int topIndex = 0;
    // bot index is the index of the next rectangle whose top should be
    // removed from interval tree (Note: bot index is for top edge of a
    // rectangle.)
    int botIndex = 0;
    while(topIndex < aListOfRectangles.size())
    {
        // bottom index is always smaller than top index because we
        // process the bottom of a rectangle before we process the top
        // of a rectangle
        if(aListOfRectangles[topIndex].y1 <= aListOfRectangles[botIndex].y2)
        {
            /*
            System.out.println("bot line: y1, x1, x2: " +
                               aListOfRectangles.get(topIndex).y1 + ", " +
                               aListOfRectangles.get(topIndex).x1 + ", " +
                               aListOfRectangles.get(topIndex).x2);
            */
            incIntervalTree(aListOfRectangles[topIndex].y1,
                                 aListOfRectangles[topIndex].x1,
                                 aListOfRectangles[topIndex].x2,
                                 aListOfRectangles[topIndex].weight,
                                 root);

            if(root->maxscore > optimalWindow->score)
            {
                optimalWindow = (Window*) root->target->clone();
                optimalWindow->score = root->maxscore;
                optimalWindow->h = aListOfRectangles[topIndex].y1;
            }
            topIndex++;

            if(root != NULL);
                /*
                System.out.println("bot local_best: " + root.target.h + ", " + root.target.l
                                   + ", " + root.target.r + ", " + root.maxscore);
                */
        }
        else
        {
            /*
            System.out.println("top line: y2, x1, x2: " +
                               aListOfRectangles.get(botIndex).y2 + ", " +
                               aListOfRectangles.get(botIndex).x1 + ", " +
                               aListOfRectangles.get(botIndex).x2);
            */
            decIntervalTree(aListOfRectangles[botIndex].y2,
                                 aListOfRectangles[botIndex].x1,
                                 aListOfRectangles[botIndex].x2,
                                 aListOfRectangles[botIndex].weight,
                                 root);

            botIndex++;
            if(root != NULL);
                //System.out.println("top local_best: " + root.target.h + ", " + root.target.l
                  //                 + ", " + root.target.r + ", " + root.maxscore);
        } // end else
    } // end while
    return optimalWindow;
}

//}



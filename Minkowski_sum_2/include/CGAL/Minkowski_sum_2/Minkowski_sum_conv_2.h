// Copyright (c) 2006  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source: $
// $Revision$ $Date$
// $Name:  $
//
// Author(s)     : Ron Wein   <wein@post.tau.ac.il>

#ifndef CGAL_MINKOWSKI_SUM_CONV_H
#define CGAL_MINKOWSKI_SUM_CONV_H

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Minkowski_sum_2/Labels.h>
#include <CGAL/Minkowski_sum_2/Arr_labeled_traits_2.h>
#include <CGAL/Minkowski_sum_2/Union_of_segment_cycles_2.h>

CGAL_BEGIN_NAMESPACE

/*! \class
 * A class for computing the Minkowski sum of two simple polygons based on the
 * convolution of their boundaries.
 */
template <class Kernel_, class Container_>
class Minkowski_sum_by_convolution_2
{
public:

  typedef Kernel_                                        Kernel;
  typedef Polygon_2<Kernel, Container_>                  Polygon_2;

private:

  // Kernel types:
  typedef typename Kernel::Point_2                       Point_2;
  typedef typename Kernel::Vector_2                      Vector_2;
  typedef typename Kernel::Direction_2                   Direction_2;
  
  // Kernel functors:
  typedef typename Kernel::Equal_2                       Equal_2;
  typedef typename Kernel::Construct_translated_point_2  Translate_point_2;
  typedef typename Kernel::Construct_vector_2            Construct_vector_2;
  typedef typename Kernel::Construct_direction_2         Construct_direction_2;
  typedef typename Kernel::Orientation_2                 Compute_orientation_2;
  typedef typename Kernel::Compare_xy_2                  Compare_xy_2;
  typedef typename Kernel::Counterclockwise_in_between_2 Ccw_in_between_2;

  // Polygon-related types:
  typedef typename Polygon_2::Vertex_circulator          Vertex_circulator;
  typedef std::pair<Vertex_circulator, unsigned int>     Vertex_ref;
  typedef std::pair<Vertex_ref, Vertex_ref>              Anchor;
  typedef std::list<Anchor>                              Anchors_queue;

  /*! \class
   * An auxiliary class for representing labels of convolved vertex pairs.
   */
  class Convolution_label
  {
  private:
    
    unsigned int      index1;       // Vertex index of the first polygon.
    unsigned int      index2;       // Vertex index of the second polygon.
    unsigned int      move_on;      // On which polygon do we move.

  public:

    /*! Default constructor. */
    Convolution_label () :
      move_on (0)
    {}

    /*! Constructor with parameters. */
    Convolution_label (unsigned int ind1, unsigned int ind2, 
		       unsigned int move) :
      index1 (ind1),
      index2 (ind2),
      move_on (move)
    {
      CGAL_precondition (move_on == 1 || move_on == 2);
    }

    /*! Less-then operator (for the usage of std::set). */
    bool operator< (const Convolution_label& label) const
    {
      if (index1 < label.index1)
	return (true);
      else if (index1 > label.index1)
	return (false);

      if (index2 < label.index2)
	return (true);
      else if (index2 > label.index2)
	return (false);

      return (move_on < label.move_on);
    }
  };

  typedef std::set<Convolution_label>                     Labels_set;

  // Traits-related types:
  typedef Arr_segment_traits_2<Kernel>                    Segment_traits_2;
  typedef Arr_labeled_traits_2<Segment_traits_2>          Traits_2; 

  typedef typename Segment_traits_2::X_monotone_curve_2   Segment_2;
  typedef typename Traits_2::X_monotone_curve_2           Labeled_segment_2;
  typedef std::list<Labeled_segment_2>                    Segments_list;

  typedef Union_of_segment_cycles_2<Traits_2, Polygon_2>  Union_2;

  // Data members:
  Equal_2                 f_equal;
  Translate_point_2       f_add;
  Construct_vector_2      f_vector;
  Construct_direction_2   f_direction;
  Compute_orientation_2   f_orientation;
  Compare_xy_2            f_compare_xy;
  Ccw_in_between_2        f_ccw_in_between;

public:

  /*! Default constructor. */
  Minkowski_sum_by_convolution_2 ()
  {
    // Obtain kernel functors.
    Kernel                ker;

    f_equal = ker.equal_2_object();
    f_add = ker.construct_translated_point_2_object(); 
    f_vector = ker.construct_vector_2_object();
    f_direction = ker.construct_direction_2_object();
    f_orientation = ker.orientation_2_object();
    f_compare_xy = ker.compare_xy_2_object();
    f_ccw_in_between = ker.counterclockwise_in_between_2_object();
  }    

  /*!
   * Compute the Minkowski sum of two simple polygons.
   * Note that as the input polygons may not be convex, the Minkowski sum may
   * not be a simple polygon. The result is therefore represented as
   * the outer boundary of the Minkowski sum (which is always a simple polygon)
   * and a container of simple polygons, representing the holes inside this
   * polygon.
   * \param pgn1 The first polygon.
   * \param pgn2 The second polygon.
   * \param sum_bound Output: A polygon respresenting the outer boundary
   *                          of the Minkowski sum.
   * \param sum_holes Output: An output iterator for the holes in the sum,
   *                          represented as simple polygons.
   * \pre Both input polygons are simple.
   * \return A past-the-end iterator for the holes in the sum.
   */
  template <class OutputIterator>
  OutputIterator operator() (const Polygon_2& pgn1,
                             const Polygon_2& pgn2,
                             Polygon_2& sum_bound,
                             OutputIterator sum_holes) const
  {
    CGAL_precondition (pgn1.is_simple());
    CGAL_precondition (pgn2.is_simple());

#ifdef RWRW_STATS
    CGAL::Timer      _timer;
    _timer.start();
#endif // RWRW_STATS

    // Prepare the vector of directions for the first polygon.
    const unsigned int    n1 = pgn1.size();
    const bool            forward1 = (pgn1.orientation() == COUNTERCLOCKWISE);
    std::vector<Direction_2>  dirs1 (n1);
    std::vector<bool>         is_reflex1 (n2);
    Vertex_circulator         prev1, curr1, next1;
    unsigned int              k1;

#ifdef RWRW_STATS
    unsigned int          ref1 = 0, ref2 = 0;
#endif // RWRW_STATS

    prev1 = next1 = curr1 = pgn1.vertices_circulator();

    if (forward1)
      --prev1;
    else
      ++prev1;

    for (k1 = 0; k1 < n1; k1++)
    {
      if (forward1)
        ++next1;
      else
        --next1;

      is_reflex1[k1] = (f_orientation (*prev1, *curr1, *next1) == RIGHT_TURN);

#ifdef RWRW_STATS
      if (is_reflex1[k1])
	ref1++;
#endif // RWRW_STATS
      
      dirs1[k1] = f_direction (f_vector (*curr1, *next1));

      prev1 = curr1;
      curr1 = next1;
    }

    // Prepare the vector of directions for the second polygon.
    // Also prepare a list containing all reflex vertices of this polygon.
    const unsigned int    n2 = pgn2.size();
    const bool            forward2 = (pgn2.orientation() == COUNTERCLOCKWISE);
    std::vector<Direction_2>  dirs2 (n2);
    std::vector<bool>         is_reflex2 (n2);
    Vertex_circulator         prev2, curr2, next2;
    Vertex_ref                bottom_left;
    bool                      is_convex2 = true;
    std::list<Vertex_ref>     reflex_vertices;
    unsigned int              k2;
    
    prev2 = next2 = curr2 = pgn2.vertices_circulator();
    
    if (forward2)
      --prev2;
    else
      ++prev2;

    for (k2 = 0; k2 < n2; k2++)
    {
      if (forward2)
        ++next2;
      else
        --next2;

      if (k2 == 0)
        bottom_left = Vertex_ref (curr2, 0);
      else if (f_compare_xy (*curr2, *(bottom_left.first)) == SMALLER)
        bottom_left = Vertex_ref (curr2, k2);

      is_reflex2[k2] = (f_orientation (*prev2, *curr2, *next2) == RIGHT_TURN);
      if (is_reflex2[k2])
      {
        // We found a reflex vertex.
        is_convex2 = false;
        reflex_vertices.push_back (Vertex_ref (curr2, k2));
      }

      dirs2[k2] = f_direction (f_vector (*curr2, *next2));
      prev2 = curr2;
      curr2 = next2;
    }

#ifdef RWRW_STATS

    ref2 = reflex_vertices.size();

#endif // RWRW_STATS

    // Add the bottom-left vertex of the second polygon to the reflex vertices.
    typename std::list<Vertex_ref>::iterator  reflex_it;

    reflex_vertices.push_front (bottom_left);

    // Construct the segments of the convolution cycles.
    unsigned int                  curr_id = 0;
    unsigned int                  cycles = 0;
    Segments_list                 conv_segments;
    Segments_list                 cycle;
    Labels_set                    used_labels;
    Anchor                        anchor;
    Anchors_queue                 queue;
    unsigned int                  loops;

    for (reflex_it = reflex_vertices.begin();
         reflex_it != reflex_vertices.end(); ++reflex_it)
    {
      // Get the current reflex vertex (or the bottom-left vertex).
      next2 = curr2 = reflex_it->first;
      k2 = reflex_it->second;

      if (forward2)
        ++next2;
      else
        --next2;

      // Search the first polygon for a vertex that contains (k1, k2, 1) in
      // a convolution cycle.
      next1 = curr1 = pgn1.vertices_circulator();
      for (k1 = 0; k1 < n1; k1++)
      {
        if (forward1)
          ++next1;
        else
          --next1;
        
        if (used_labels.count (Convolution_label (k1, k2, 1)) == 0 &&
            (f_ccw_in_between (dirs1[k1], 
                               dirs2[(n2 + k2 - 1) % n2],
                               dirs2[k2]) ||
             f_equal (dirs1[k1], dirs2[k2])))
        {
          // Construct the current convolution cycle.
          queue.clear();
          queue.push_back (Anchor (Vertex_ref (curr1, k1),
                                   Vertex_ref (curr2, k2)));
          loops = 0;

          while (! queue.empty())
          {
            // Pop the first pair of anchor vertices from the queue.
            anchor = queue.front();
            queue.pop_front();
            loops++;

            const Vertex_ref&    vert1 = anchor.first;
            const Vertex_ref&    vert2 = anchor.second;

            if (loops > 0 &&
                used_labels.count (Convolution_label (vert1.second, 
                                                      vert2.second, 2)) != 0)
            {
              loops--;
              continue;
            }

            // Add a loop to the current convolution cycle.
            curr_id++;
            _convolution_cycle (curr_id,
                                n1, forward1, dirs1, is_reflex1,
                                vert1.first, vert1.second,
                                n2, forward2, dirs2, is_reflex2,
                                vert2.first, vert2.second,
                                used_labels, queue,
                                cycle);

            // Catenate the segments of the current loop to the convolution
            // list.
            if (cycle.empty())
            {
              loops--;
            }
            else
            {
              conv_segments.splice (conv_segments.end(), cycle);
              CGAL_assertion (cycle.empty());
            }
          }
	  cycles++;

#ifdef RWRW_STATS

          std::cout << "Cycle no. " << cycles
                    << " containing " << conv_segments.size()
                    << " segments (in " << loops << " loops)." << std::endl;

#endif // RWRW_STATS
        }

        curr1 = next1;
      }
    }

    // RWRW: Output:
    /* RWRW - DELETE THIS:
    std::ofstream    ofs ("conv_segments.txt");

    ofs << conv_segments.size() << std::endl;
    
    typename Segments_list::const_iterator  sit;

    for (sit = conv_segments.begin(); sit != conv_segments.end(); ++sit)
      ofs << sit->source() << "  " << sit->target() << std::endl;

    ofs.close();
    */

#ifdef RWRW_STATS

    std::cout << "|P| = " << n1 << " (" << ref1
	      << ")   |Q| = " << n2 << " (" << ref2 << ")" << std::endl;
    _timer.stop();
    std::cout << cycles << " cycles, "
	      << conv_segments.size() << " segments" << std::endl;
    std::cout << "Computing the convolution took "
	      << _timer.time() << " seconds. " << std::endl;

#endif // RWRW_STATS

    // Compute the union of the cycles that represent the Minkowski sum.
    Union_2     unite;

    sum_holes = unite (conv_segments.begin(), conv_segments.end(),
                       sum_bound, sum_holes);

    return (sum_holes);
  }

private:

  /*!
   * Compute a convolution cycle starting from tow given vertices.
   * \param cycle_id The index of the current cycle.
   * \param n1 The size of the first polygon.
   * \param forward1 Whether we move forward or backward on this polygon.
   * \param dirs1 The directions of the edges in the first polygon.
   * \param curr1 Points to the current vertex in the first polygon.
   * \param k1 The index of this vertex (between 0 and n1 - 1).
   * \param n2 The size of the second polygon.
   * \param forward2 Whether we move forward or backward on this polygon.
   * \param dirs2 The directions of the edges in the second polygon.
   * \param curr2 Points to the current vertex in the second polygon.
   * \param k2 The index of this vertex (between 0 and n2 - 1).
   * \param used_labels Input/output: The segment labels used so far.
   * \param queue A queue of anchor vertices for loops in the cycle.
   * \param cycle Output: An list of labeled segments that constitute the
   *                      convolution cycle.
   * \return A past-the-end iterator for the output segments.
   */
  void _convolution_cycle (unsigned int cycle_id,
                           unsigned int n1, bool forward1, 
                           const std::vector<Direction_2>& dirs1,
                           const std::vector<bool>& is_reflex1,
                           Vertex_circulator curr1, unsigned int k1,
                           unsigned int n2, bool forward2, 
                           const std::vector<Direction_2>& dirs2, 
                           const std::vector<bool>& is_reflex2,
                           Vertex_circulator curr2, unsigned int k2,
                           Labels_set& used_labels,
                           Anchors_queue& queue,
                           Segments_list& cycle) const

  {
    // Update the circulator pointing to the next vertices in both polygons.
    unsigned int          seg_index = 0;
    const unsigned int    first1 = k1;
    const unsigned int    first2 = k2;
    Vertex_circulator     next1 = curr1;
    Vertex_circulator     next2 = curr2;
    Convolution_label     label;
    Point_2               first_pt, curr_pt, next_pt;
    bool                  inc1, inc2;
    Comparison_result     res;
    const bool            MOVE_ON_1 = true, MOVE_ON_2 = false;

    if (forward1)
      ++next1;
    else
      --next1;

    if (forward2)
      ++next2;
    else
      --next2;
    
    // Start constructing the convolution cycle from *curr1 and *curr2.
    curr_pt = first_pt = f_add (*curr1, f_vector(CGAL::ORIGIN, *curr2));
    do
    {
      // Determine on which polygons we should move.
      inc1 = false;
      inc2 = false;

      if (f_ccw_in_between (dirs1[k1],
                            dirs2[(n2 + k2 - 1) % n2],
                            dirs2[k2]))
      {
        inc1 = true;

        label = Convolution_label (k1, k2, 1);
        if (used_labels.count (label) != 0)
          inc1 = false;
      }
      
      if (f_ccw_in_between (dirs2[k2],
                            dirs1[(n1 + k1 - 1) % n1],
                            dirs1[k1]))
      {
        if (inc1)
        {
          // If we are about to increment the first polygon, add an anchor
          // to the queue. Next time we reach here we will increment the
          // second polygon (and proceed until reaching this point again and
          // closing the loop).
          label = Convolution_label (k1, k2, 2);
          if (used_labels.count (label) == 0)
          {
	    queue.push_back (Anchor (Vertex_ref (curr1, k1),
				     Vertex_ref (curr2, k2)));
          }
        }
        else
        {
          inc2 = true;

          label = Convolution_label (k1, k2, 2);
          if (used_labels.count (label) != 0)
            inc2 = false;
        }
      }
      
      if (! inc1 && ! inc2 &&
          f_equal (dirs1[k1], dirs2[k2]))
      {
        inc1 = true;
        inc2 = true;

        label = Convolution_label (k1, k2, 1);
        if (used_labels.count (label) != 0)
          inc1 = false;

        if (inc1)
          label = Convolution_label ((k1 + 1) % n1, k2, 2); 
        else
          label = Convolution_label (k1, k2, 2);

        if (used_labels.count (label) != 0)
          inc2 = false;
      }

      CGAL_assertion (inc1 || inc2);

      // Act according to the increment flags.
      if (inc1)
      {
        // Translate the current edge of the first polygon to *curr2.
        next_pt = f_add (*next1, f_vector(CGAL::ORIGIN, *curr2));

        res = f_compare_xy (curr_pt, next_pt);
        CGAL_assertion (res != EQUAL);

        //@!@
        if (! is_reflex2[k2])
        {
          cycle.push_back (Labeled_segment_2 (Segment_2 (curr_pt, next_pt),
                                              X_curve_label ((res == SMALLER),
                                                             cycle_id,
                                                             seg_index,
                                                             MOVE_ON_1)));
        }
        used_labels.insert (Convolution_label (k1, k2, 1));
	seg_index++;

        // Proceed to the next vertex of the first polygon.
        curr1 = next1;
        k1 = (k1 + 1) % n1;

        if (forward1)
          ++next1;
        else
          --next1;

        curr_pt = next_pt;
      }

      if (inc2)
      {
        // Translate the current edge of the second polygon to *curr1.
        next_pt = f_add (*next2, f_vector(CGAL::ORIGIN, *curr1));

        res = f_compare_xy (curr_pt, next_pt);
        CGAL_assertion (res != EQUAL);

        //@!@
        if (! is_reflex1[k1])
        {
          cycle.push_back (Labeled_segment_2 (Segment_2 (curr_pt, next_pt),
                                              X_curve_label ((res == SMALLER),
                                                             cycle_id,
                                                             seg_index,
                                                             MOVE_ON_2)));
        }
        used_labels.insert (Convolution_label (k1, k2, 2));
	seg_index++;

        // Proceed to the next vertex of the second polygon.
        curr2 = next2;
        k2 = (k2 + 1) % n2;

        if (forward2)
          ++next2;
        else
          --next2;

        curr_pt = next_pt;
      }

    } while (k1 != first1 || k2 != first2); 

    CGAL_assertion (f_equal (curr_pt, first_pt));

    /*
    // Before moving un-necessary sub-cycles from the segment list, make sure
    // the list contains no "cyclic" sub-cylces. We do that by making sure that
    // the first and last segments of the list correspond to traversals of
    // different polygons.
    int     steps = cycle.size();

    while (steps > 0 &&
           cycle.front().label().get_flag() == 
           cycle.back().label().get_flag())
    {
      cycle.push_back (cycle.front());
      cycle.pop_front();
      steps--;
    }

    if (steps == 0)
    {
      cycle.clear();
      return;
    }

    // Reduce un-necessary sub-cycles. This is done by scanning the segments
    // list for subsequences sharing a common move_on indicator. When we
    // encounter such a subsequence that equals the size of the corresponding
    // polygon, we can safely remove it from the convolution cycle.
    typename std::list<Labeled_segment_2>::iterator  first, curr;
    bool                                             move_on;
    unsigned int                                     count = 1;
    bool                                             reduced_cycle = false;

    curr = first = cycle.begin();
    move_on = first->label().get_flag();

    curr->label().set_flag (false);
    ++curr;
    while (curr != cycle.end())
    {
      if ((move_on == MOVE_ON_1 && count == n1) ||
          (move_on == MOVE_ON_2 && count == n2))
      {
	// We have discovered a sequence of moves on one of the polygon that
	// equals the polygon size, so we can remove this sequence.
        cycle.erase (first, curr);
	reduced_cycle = true;
        first = curr;
        move_on = first->label().get_flag();
        count = 1;
      }
      else
      {
        if (move_on == curr->label().get_flag())
        {
          count++;
        }
        else
        {
          first = curr;
          move_on = first->label().get_flag();
          count = 1;
        }
      }

      curr->label().set_flag (false);
      ++curr;
    }

    if ((move_on == MOVE_ON_1 && count == n1) ||
        (move_on == MOVE_ON_2 && count == n2))
    {
      cycle.erase (first, curr);
      reduced_cycle = true;
    }

    if (reduced_cycle)
    {
      // In case we have reduced the cycle, re-number the segments in it.
      seg_index = 0;
      for (curr = cycle.begin(); curr != cycle.end(); ++curr, ++seg_index)
        cycle.back().label().set_index (seg_index);
    }
    */

    if (cycle.back().label().index() + 1 == seg_index)
      cycle.back().label().set_flag (true);

    return;
  }
};


CGAL_END_NAMESPACE

#endif

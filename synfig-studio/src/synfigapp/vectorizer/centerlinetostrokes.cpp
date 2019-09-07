/* === S Y N F I G ========================================================= */
/*!	\file centerlinetostrokes.cpp
**	\brief centerlinetostrokes
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#include "polygonizerclasses.h"
#include <synfig/valuenodes/valuenode_bline.h>
#include <synfig/blinepoint.h>
#include <synfig/layer.h>
#include <synfig/canvas.h>
#include <synfig/valuenode.h>


/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
const double Polyg_eps_max = 1;     // Sequence simplification max error
const double Polyg_eps_mul = 0.75;  // Sequence simpl. thickness-multiplier error
const double Quad_eps_max =  infinity;  // As above, for sequence conversion into strokes
synfig::CanvasHandle canvas;
synfig::Point topleft(0,0),bottomright(0,0);
bool max_thickness_zero = false;
/* === P R O C E D U R E S ================================================= */

etl::handle<synfig::Layer> BezierToOutline(studio::PointList segment)
{
  int segment_size = segment.size();
  synfig::Layer::Handle layer(synfig::Layer::create("outline"));
  std::vector<synfig::BLinePoint> bline_point_list; 
  synfig::Point a = canvas->rend_desc().get_br();
  synfig::Point b = canvas->rend_desc().get_tl();
  synfig::Point q = a - b;
  float p = canvas->rend_desc().get_w();
  //std::cout<<"This is canvas TL: ("<<b[0]<<", "<<b[1]<<"), ("<<a[0]<<", "<<a[1]<<")\n";
  float unit_size = p/q[0];
  //std::cout<<"This is getw(): "<<p<<", Unit size:"<<unit_size<<"\n";
  float multiplier = unit_size/60.0;
  // here fitting and shifting happen
  for(int i=0;i<segment_size;i++)
  {
    segment[i][0] = multiplier * segment[i][0]/unit_size + topleft[0];//x from TL;
    segment[i][1] = multiplier * segment[i][1]/unit_size + bottomright[1];// y from BR;
    segment[i][2] = segment[i][2]/2;
  }

  if(max_thickness_zero)
    for(int i=0;i<segment_size;i++)
      segment[i][2] = 1.0;
     
  switch(segment_size)// in any case size>=3
  {
    case 3:{//std::cout<<"This is case 3\n";
            bline_point_list.push_back(synfig::BLinePoint()); 
            bline_point_list.push_back(synfig::BLinePoint()); 
            bline_point_list[0].set_vertex(segment[0].to_2d());
            bline_point_list[1].set_vertex(segment[2].to_2d());
            bline_point_list[0].set_tangent((segment[1].to_2d() - segment[0].to_2d()) * 2);
            bline_point_list[1].set_tangent((segment[2].to_2d() - segment[1].to_2d()) * 2);
            bline_point_list[0].set_width(segment[0][2]);
            bline_point_list[1].set_width(segment[2][2]); 
            
    }break;

    case 4:{   
            bline_point_list.push_back(synfig::BLinePoint()); 
            bline_point_list.push_back(synfig::BLinePoint()); 
            bline_point_list[0].set_vertex(segment[0].to_2d());
            bline_point_list[1].set_vertex(segment[3].to_2d());
            bline_point_list[0].set_tangent((segment[1].to_2d() - segment[0].to_2d()) * 2);
            bline_point_list[1].set_tangent((segment[3].to_2d() - segment[2].to_2d()) * 2);
            bline_point_list[0].set_width(segment[0][2]);
            bline_point_list[1].set_width(segment[3][2]); 
            
    }break;

    default:{/*Odd : 1 2 3 , 3 4 5, 5 6 7, 7 8 9
                Even : 1 2 3 4, 4 5 6, 6 7 8 */
                int num =0,point =0;
                if(segment_size & 1)
                {
                  bline_point_list.push_back(synfig::BLinePoint()); 
                  bline_point_list.push_back(synfig::BLinePoint()); 
                  bline_point_list[0].set_vertex(segment[0].to_2d());// first point 
                  bline_point_list[1].set_vertex(segment[2].to_2d());// second point
                  bline_point_list[0].set_tangent((segment[1].to_2d() - segment[0].to_2d()) * 2);
                  bline_point_list[1].set_tangent1((segment[2].to_2d() - segment[1].to_2d()) * 2);
                  bline_point_list[0].set_width(segment[0][2]);
                  bline_point_list[1].set_width(segment[2][2]);
                  num = 2;// represent segment index used
                }
                else
                {
                  bline_point_list.push_back(synfig::BLinePoint()); 
                  bline_point_list.push_back(synfig::BLinePoint()); 
                  bline_point_list[0].set_vertex(segment[0].to_2d());// first point 
                  bline_point_list[1].set_vertex(segment[3].to_2d());// second point
                  bline_point_list[0].set_tangent((segment[1].to_2d() - segment[0].to_2d()) * 2);
                  bline_point_list[1].set_tangent1((segment[3].to_2d() - segment[2].to_2d()) * 2);
                  bline_point_list[0].set_width(segment[0][2]);
                  bline_point_list[1].set_width(segment[3][2]); 
                  num = 3;// represent segment index used
                }
              
                for (num,point = 2; num < segment_size - 3;point++, num += 2) 
                { 
                  bline_point_list.push_back(synfig::BLinePoint()); 
                  bline_point_list[point].set_vertex(segment[num+2].to_2d());// last point
                  bline_point_list[point].set_width(segment[num+2][2]);// last point
                  bline_point_list[point - 1].set_tangent2((segment[num+1].to_2d() - segment[num].to_2d()) * 2);
                  bline_point_list[point].set_tangent1((segment[num+2].to_2d() - segment[num+1].to_2d()) * 2);
                }
                // add last blinepoint and set it's param
                bline_point_list.push_back(synfig::BLinePoint());
                bline_point_list.back().set_vertex(segment[segment_size-1].to_2d());
                bline_point_list.back().set_width(segment[segment_size-1][2]);
                bline_point_list.back().set_tangent((segment[segment_size-1].to_2d()-segment[segment_size-2].to_2d())*2);
    }break;

  }
  
  etl::handle<synfig::ValueNode_BLine> bline_value_node; 
  etl::handle<synfig::ValueNode_DynamicList> value_node;
  etl::handle<synfig::ValueNode> vn;

	vn=value_node=bline_value_node=synfig::ValueNode_BLine::create(bline_point_list, canvas);
  layer->connect_dynamic_param("bline",vn);
  layer->set_param("width",5.0/unit_size);
  return layer;
}

/* === M E T H O D S ======================================================= */
inline double distance2(const synfig::Point3 &P, const synfig::Point3 &v, const synfig::Point3 &B) 
{
  double t    = P * v - B * v;
  synfig::Point3 Q = B + (v * t) - P;

  return Q * Q;
}

inline double tdistance(synfig::Point3 P, synfig::Point3 v, synfig::Point3 B) {
  double vv = v.mag_squared();
  if (vv < 0.01) return -1;

  double t = (P * v - B * v) / vv;
  synfig::Point3 Q = B + v * t - P;

  return Q.mag();
}

/* === E N T R Y P O I N T ================================================= */
class SequenceSimplifier 
{
  const Sequence *m_s;
  const SkeletonGraph *m_graph;

private:
  class Length 
  {
  public:
    int n;
    double l;
    UINT firstNode, secondNode;

    Length() : n(0), l(0) {}
    Length(int n_, double l_) : n(n_), l(l_) {}

    inline void infty(void) 
    {
      n = infinity;
      l = infinity;
    }
    inline bool operator<(Length sl) 
    {
      return n < sl.n ? 1 : n > sl.n ? 0 : l < sl.l ? 1 : 0;
    }
    inline Length operator+(Length sl) { return Length(n + sl.n, l + sl.l); }
  };

  Length lengthOf(UINT a, UINT aLink, UINT b);

public:
  // Methods
  SequenceSimplifier(const Sequence *s) : m_s(s), m_graph(m_s->m_graphHolder) {}

  void simplify(std::vector<unsigned int> &result);
};

//--------------------------------------------------------------------------

// Bellman algorithm for Sequences
// NOTE: Circular Sequences are dealt.
void SequenceSimplifier::simplify(std::vector<unsigned int> &result) {
  // Initialize variables
  unsigned int n;
  unsigned int i, j, iLink, jLink;

  // NOTE: If s is circular, we have to protect

  i     = m_s->m_head;
  iLink = m_s->m_headLink;
  // NOTE: If m_head==m_tail then we have to force the first step by "|| n==1"
  for (n = 1; i != m_s->m_tail || n == 1; ++n, m_s->next(i, iLink))
    ;

  Length L_att, L_min, l_min, l_ji;
  unsigned int p_i, a, b;

  std::vector<Length> M(n);
  std::vector<Length> K(n);
  std::vector<unsigned int> P(n);

  // Search for minimal path
  i     = m_s->m_head;
  iLink = m_s->m_headLink;
  for (a = 1; i != m_s->m_tail || a == 1; m_s->next(i, iLink), ++a) 
  {
    L_min.infty();
    l_min.infty();
    p_i = 0;

    j                  = m_s->m_head;
    jLink              = m_s->m_headLink;
    unsigned int iNext = m_graph->getNode(i).getLink(iLink).getNext();
    for (b = 0; j != iNext || b == 0; m_s->next(j, jLink), ++b) 
    {
      if ((L_att = M[b] + (l_ji = lengthOf(j, jLink, iNext))) < L_min) 
      {
        L_min = L_att;
        p_i   = b;
        l_min = l_ji;
      }
    }
    M[a] = L_min;
    K[a] = l_min;
    P[a] = p_i;
  }

  // Copies minimal path found to the output reducedIndices vector
  // NOTE: size() is added due to circular sequences case handling
  unsigned int redSize = result.size();

  result.resize(redSize + M[n - 1].n + 1);

  result[redSize + M[n - 1].n] = K[n - 1].secondNode;
  for (b = n - 1, a = redSize + M[n - 1].n - 1; b > 0; b = P[b], --a)
    result[a] = K[b].firstNode;
}

//--------------------------------------------------------------------------

// Length between two sequence points
SequenceSimplifier::Length SequenceSimplifier::lengthOf(UINT a, UINT aLink, UINT b) 
{
  UINT curr, old;
  synfig::Point3 v;
  double d, vv;
  Length res;

  res.n          = 1;
  res.firstNode  = a;
  res.secondNode = b;

  v  = *m_graph->getNode(b) - *m_graph->getNode(a);
  vv = v.mag();

  curr = m_graph->getNode(a).getLink(aLink).getNext();
  old  = a;

  // If the distance between extremities is small, check if the same holds
  // for internal points; if so, ok - otherwise set infty().
  if (vv < 0.1) 
  {
    for (; curr != b; m_s->advance(old, curr)) 
    {
      d = (*m_graph->getNode(curr) - *m_graph->getNode(a)).mag();// distance btw points
      if (d > 0.1) res.infty();
    }
    return res;
  }

  // Otherwise, check distances from line passing from a and b
  v = v * (1 / vv);

  for (; curr != b; m_s->advance(old, curr)) 
  {
    d = distance2(*m_graph->getNode(curr), v, *m_graph->getNode(a));
    if (d > std::min(m_graph->getNode(curr)->operator[](2) * Polyg_eps_mul, Polyg_eps_max)) 
    {
      res.infty();
      return res;
    } 
    else
      res.l += d;
  }

  return res;
}

//==========================================================================

//===============================
//      Sequence conversion
//===============================

// EXPLANATION: Sequences convert into Outline by applying a SequenceConverter
//  class. A graph minimal-path algorithm is run by using a
//  lexicographic-ordered
//  (number of quadratics, error) length.

class SequenceConverter 
{
  const Sequence *m_s;
  const SkeletonGraph *m_graph;

  double m_penalty;

public:
  // Length construction globals (see 'lengthOf' method)
  unsigned int middle;
  std::vector<double> pars;

  class Length 
  {
  public:
    int n;
    double l;
    std::vector<synfig::Point3> CPs;

    Length() : n(0), l(0) {}
    Length(int n_, double l_) : n(n_), l(l_) {}

    inline void infty(void) 
    {
      n = infinity;
      l = infinity;
    }

    inline bool operator<(Length sl) 
    {
      return n < sl.n ? 1 : n > sl.n ? 0 : l < sl.l ? 1 : 0;
    }

    inline Length operator+(Length sl) { return Length(n + sl.n, l + sl.l); }

    void set_CPs(const synfig::Point3 &a, const synfig::Point3 &b, const synfig::Point3 &c) 
    {
      CPs.resize(3);
      CPs[0] = a;
      CPs[1] = b;
      CPs[2] = c;
    }

    void set_CPs(const synfig::Point3 &a, const synfig::Point3 &b, const synfig::Point3 &c,
                 const synfig::Point3 &d, const synfig::Point3 &e) 
    {
      CPs.resize(5);
      CPs[0] = a;
      CPs[1] = b;
      CPs[2] = c;
      CPs[3] = d;
      CPs[4] = e;
    }
  };

  // Intermediate Sequence form
  std::vector<synfig::Point3> middleAddedSequence;
  std::vector<unsigned int> *inputIndices;

  // Methods
  SequenceConverter(const Sequence *s, double penalty)
      : m_s(s), m_graph(m_s->m_graphHolder), m_penalty(penalty) {}

  Length lengthOf(unsigned int a, unsigned int b);
  void addMiddlePoints();
  etl::handle<synfig::Layer> operator()(std::vector<unsigned int> *indices);

  // Length construction methods
  bool parametrize(unsigned int a, unsigned int b);
  void lengthOfTriplet(unsigned int i, Length &len);
  bool calculateCPs(unsigned int i, unsigned int j, Length &len);
  bool penalty(unsigned int a, unsigned int b, Length &len);
};

//--------------------------------------------------------------------------

// Changes in stroke thickness are considered more penalizating
inline double ellProd(const synfig::Point3 &a, const synfig::Point3 &b) {
  return a[0] * b[0] + a[1] * b[1] + 5 * a[2] * b[2];
}

//--------------------------------------------------------------------------

// EXPLANATION:  After simplification, we receive a vector<UINT> of indices
// corresponding to the vertices of the simplified current sequence.
// Before beginning conversion, we need to add middle points between the
// above vertex points.

inline void SequenceConverter::addMiddlePoints() 
{
  unsigned int i, j, n;

  n = inputIndices->size();
  middleAddedSequence.clear();

  if (n == 2) 
  {
    middleAddedSequence.resize(3);
    middleAddedSequence[0] = *m_graph->getNode((*inputIndices)[0]);
    middleAddedSequence[1] = (*m_graph->getNode((*inputIndices)[0]) +
                              *m_graph->getNode((*inputIndices)[1])) * 0.5;
    middleAddedSequence[2] = *m_graph->getNode((*inputIndices)[1]);
  } 
  else 
  {
    middleAddedSequence.resize(2 * n - 3);
    middleAddedSequence[0] = *m_graph->getNode((*inputIndices)[0]);
    
    for (i = j = 1; i < n - 2; ++i, j += 2) 
    {
      middleAddedSequence[j]     = *m_graph->getNode((*inputIndices)[i]);
      middleAddedSequence[j + 1] = (*m_graph->getNode((*inputIndices)[i]) +
                                    *m_graph->getNode((*inputIndices)[i + 1])) * 0.5;
    }
    middleAddedSequence[j]     = *m_graph->getNode((*inputIndices)[n - 2]);
    middleAddedSequence[j + 1] = *m_graph->getNode((*inputIndices)[n - 1]);
  }
}

//--------------------------------------------------------------------------

etl::handle<synfig::Layer> SequenceConverter::operator()(std::vector<unsigned int> *indices) {
  // Prepare Sequence
  inputIndices = indices;
  addMiddlePoints();

  // Initialize local variables
  unsigned int n = (middleAddedSequence.size() + 1) / 2;  // Number of middle points

  unsigned int i;
  int j;

  Length L_att, L_min, l_min, l_ji;
  unsigned int p_i, a, b;

  std::vector<Length> M(n);
  std::vector<Length> K(n);
  std::vector<unsigned int> P(n);

  // Bellman algorithm
  for (i = 2, a = 1; i < middleAddedSequence.size(); i += 2, ++a) 
  {
    L_min.infty();
    l_min.infty();
    p_i = 0;

    for (j = i - 2, b = j / 2; j >= 0; j -= 2, --b) 
    {
      if ((L_att = M[b] + (l_ji = lengthOf(j, i))) < L_min) 
      {
        L_min = L_att;
        p_i   = b;
        l_min = l_ji;
      }
      // NOTE: The following else may be taken out to perform a deeper
      // search for optimal result. However, it prevents quadratic complexities
      // on large-scale images.
      else if (l_ji.n == infinity)
        break;  // Stops searching for current i
    }
    M[a] = L_min;
    K[a] = l_min;
    P[a] = p_i;
  }

  // Read off the output
  std::vector<synfig::Point3> controlPoints(2 * M[n - 1].n + 1);

  for (b = n - 1, a = 2 * M[n - 1].n; b > 0; b = P[b]) 
  {
    for (i = K[b].CPs.size() - 1; i > 0; --i, --a)
      controlPoints[a] = K[b].CPs[i];
  }
  controlPoints[0] = middleAddedSequence[0];
  
  etl::handle<synfig::Layer> res = BezierToOutline(controlPoints);

  return res;
}

//--------------------------------------------------------------------------

//--------------------------------------
//      Conversion Length build-up
//--------------------------------------

SequenceConverter::Length SequenceConverter::lengthOf(unsigned int a, unsigned int b) 
{
  Length l;

  // If we have a triplet, apply a specific procedure
  if (b == a + 2) 
  {
    lengthOfTriplet(a, l);
    return l;
  }
  // otherwise
  if (!parametrize(a, b) || !calculateCPs(a, b, l) || !penalty(a, b, l))
    l.infty();
  return l;
}

//--------------------------------------------------------------------------

void SequenceConverter::lengthOfTriplet(unsigned int i, Length &len) 
{
  synfig::Point3 A = middleAddedSequence[i];
  synfig::Point3 B = middleAddedSequence[i + 1];
  synfig::Point3 C = middleAddedSequence[i + 2];

  // We assume that this convertion is faithful, avoiding length penalty
  len.l = 0;
  double d = tdistance(B, C - A, A);
  
  if (d <= 2) 
  {
    len.n = 1;
    len.set_CPs(A, B, C);
  } 
  else if (d <= 6) 
  {
    len.n       = 2;
    d           = (d - 1) / d;
    synfig::Point3 U = A + (B - A) * d, V = C + (B - C) * d;
    len.set_CPs(A, U, (U + V) * 0.5, V, C);
  } 
  else 
  {
    len.n = 2;
    len.set_CPs(A, (A + B) * 0.5, B, (B + C) * 0.5, C);
  }
}

//--------------------------------------------------------------------------

bool SequenceConverter::parametrize(unsigned int a, unsigned int b) 
{
  unsigned int curr, old;
  unsigned int i;
  double w, t;
  double den;

  pars.clear();
  pars.push_back(0);

  for (old = a, curr = a + 1, den = 0; curr < b; old = curr, curr += 2) 
  {
    w = (middleAddedSequence[curr] - middleAddedSequence[old]).mag();
    den += w;
    pars.push_back(w);
  }
  w = (middleAddedSequence[b] - middleAddedSequence[old]).mag();
  den += w;
  pars.push_back(w);

  if (den < 0.1) return 0;

  for (i = 1, t = 0; i < pars.size(); ++i) 
  {
    t += 2 * pars[i] / den;
    pars[i] = t;
  }

  // Seek the interval which holds 1 - the middle interval
  for (middle = 0; middle < pars.size() && pars[middle + 1] <= 1; ++middle)
    ;

  return 1;
}

//==========================================================================

//------------------------
//    CP construcion
//------------------------

// NOTE: Check my thesis for variable meanings (int_ stands for 'integral').

// Some integrals (int_) for the CP linear system resolution

inline synfig::Point3 int_H(const synfig::Point3 &A, const synfig::Point3 &B, double t1, double t2) 
{
  return  B * -(0.375 * (pow(t2, 4) - pow(t1, 4))) +
         (B * 0.6667 - A * 0.5) * (pow(t2, 3) - pow(t1, 3)) +
         A * (pow(t2, 2) - pow(t1, 2));
}

//--------------------------------------------------------------------------

inline synfig::Point3 int_K(const synfig::Point3 &A, const synfig::Point3 &B, double t1, double t2) 
{
  return (B * 0.125) * (pow(t2, 4) - pow(t1, 4)) +
         (A * 0.1667) * (pow(t2, 3) - pow(t1, 3));
}

//--------------------------------------------------------------------------

bool SequenceConverter::calculateCPs(unsigned int i, unsigned int j, Length &len) 
{
  unsigned int curr, old;

  synfig::Matrix M;
  synfig::Point l;
  synfig::Point3 a, e, x, y, A, B;
  synfig::Point3 IH, IK, IM, IN_;  //"IN" seems to be reserved word
  double HxL, KyL, MxO, NyO;
  unsigned int k;
  a = middleAddedSequence[i];
  e = middleAddedSequence[j];
  x = middleAddedSequence[i + 1] - a;
  y = middleAddedSequence[j - 1] - e;

  // Build TAffine M
  double par = ellProd(x, y) / 5;
  M = synfig::Matrix( synfig::Point(ellProd(x, x)/3, par), synfig::Point(par, ellProd(y, y)/3),synfig::Point() );

   // Integral from 0.0 to 1.0
  for (k = 0, old = i, curr = i + 1; k < middle; ++k, old = curr, curr += 2) 
  {
    B = (middleAddedSequence[curr] - middleAddedSequence[old]) *
        (1 / (pars[k + 1] - pars[k]));
    A = middleAddedSequence[old] - B * pars[k];
    IH += int_H(A, B, pars[k], pars[k + 1]);
    IK += int_K(A, B, pars[k], pars[k + 1]);
  }

  if (curr == j + 1) curr = j;
  B = (middleAddedSequence[curr] - middleAddedSequence[old]) *
      (1 / (pars[k + 1] - pars[k]));
  A = middleAddedSequence[old] - B * pars[k];
  IH += int_H(A, B, pars[k], 1.0);
  IK += int_K(A, B, pars[k], 1.0);

  // Integral from 1.0 to 2.0
  for (k = pars.size() - 1, old = j, curr = j - 1; k > middle + 1;
       --k, old = curr, curr -= 2) {
    B = (middleAddedSequence[curr] - middleAddedSequence[old]) *
        (1 / (pars[k] - pars[k - 1]));
    A = middleAddedSequence[curr] - B * (2 - pars[k - 1]);
    IM += int_K(A, B, 2 - pars[k], 2 - pars[k - 1]);
    IN_ += int_H(A, B, 2 - pars[k], 2 - pars[k - 1]);
  }

  if (old == i + 1) curr = i;
  B = (middleAddedSequence[curr] - middleAddedSequence[old]) *
      (1 / (pars[k] - pars[k - 1]));
  A = middleAddedSequence[curr] - B * (2 - pars[k - 1]);
  IM += int_K(A, B, 2 - pars[k], 1.0);
  IN_ += int_H(A, B, 2 - pars[k], 1.0);

  // Polygonal-free integrals
  synfig::Point3 f = (a + e) * 0.5;
  HxL         = (ellProd(a, x) * 0.3) + (ellProd(f, x) / 5.0);
  NyO         = (ellProd(e, y) * 0.3) + (ellProd(f, y) / 5.0);
  KyL         = (ellProd(a, y) / 15.0) + (ellProd(f, y) / 10.0);
  MxO         = ((e * x) / 15.0) + (ellProd(f, x) / 10.0);

  // Infine, ho il termine noto
  l = synfig::Point(ellProd(IH, x) - HxL + ellProd(IM, x) - MxO,
              ellProd(IK, y) - KyL + ellProd(IN_, y) - NyO);
  M.m20 = -l[0];
  M.m21 = -l[1];

  // Check validity conditions:
  //  a) System is not singular
  if (fabs(M.det()) < 0.01) return 0;

  M.invert();

  //  b) Shift (solution) is positive
  if (M.m20 < 0 || M.m21 < 0) return 0;
  synfig::Point3 b = a + x * M.m20;
  synfig::Point3 d = e + y * M.m21;

  //  c) The height of every CP must be >=0
  if (b[2] < 0 || d[2] < 0) return 0;
  len.set_CPs(a, b, (b + d) * 0.5, d, e);

  return 1;
}

//==========================================================================

//------------------------
//      Penalties
//------------------------

inline synfig::Point3 int_B0a(const synfig::Point3 &A, const synfig::Point3 &B, double t1,
                         double t2) {
  return B * (0.25 * (pow(t2, 4) - pow(t1, 4))) +
         (A - B * 2.0) * ((pow(t2, 3) - pow(t1, 3)) / 3.0) +
         (B - A * 2.0) * (0.5 * (pow(t2, 2) - pow(t1, 2))) + A * (t2 - t1);
}

//--------------------------------------------------------------------------

inline synfig::Point3 int_B1a(const synfig::Point3 &A, const synfig::Point3 &B, double t1,
                         double t2) {
return B * -(0.5 * (pow(t2, 4) - pow(t1, 4))) + 
          ((B - A) * 2.0 * ((pow(t2, 3) - pow(t1, 3)) / 3.0) +
          A * (pow(t2, 2) - pow(t1, 2)));
}

//--------------------------------------------------------------------------

inline synfig::Point3 int_B2a(const synfig::Point3 &A, const synfig::Point3 &B, double t1,
                         double t2) {
  return B * (0.25 * (pow(t2, 4) - pow(t1, 4))) +
         A * ((pow(t2, 3) - pow(t1, 3)) / 3.0);
}

//--------------------------------------------------------------------------

inline double int_a2(const synfig::Point3 &A, const synfig::Point3 &B, double t1,
                     double t2) {
  return ellProd(A, A) * (t2 - t1) + ellProd(A, B) * (pow(t2, 2) - pow(t1, 2)) +
         (ellProd(B, B) * (pow(t2, 3) - pow(t1, 3)) / 3.0);
}

//--------------------------------------------------------------------------

// Penalty is the integral of the square norm of differences between polygonal
// and quadratics.
bool SequenceConverter::penalty(unsigned int a, unsigned int b, Length &len) 
{
  unsigned int curr, old;

  const std::vector<synfig::Point3> &CPs = len.CPs;
  synfig::Point3 A, B, P0, P1, P2;
  double p, p_max;
  unsigned int k;

  len.n = 2;  // A couple of arcs

  // Prepare max penalty p_max
  p_max = 0;
  for (curr = a + 1, old = a, k = 0; curr < b; ++k, old = curr, curr += 2) 
  {
    p_max += (middleAddedSequence[curr][2] + middleAddedSequence[old][2]) *
             (pars[k + 1] - pars[k]) / 2;
  }
  p_max += (middleAddedSequence[b][2] + middleAddedSequence[old][2]) *
           (pars[k + 1] - pars[k]) / 2;

  // Confronting 4th power of error with mean polygonal thickness
  // - can be changed
  p_max = std::min(sqrt(p_max) * m_penalty, Quad_eps_max);

  // CP only integral
  p = (ellProd(CPs[0], CPs[0]) + 2 * ellProd(CPs[2], CPs[2]) +
       ellProd(CPs[4], CPs[4]) + ellProd(CPs[0], CPs[1]) +
       ellProd(CPs[1], CPs[2]) + ellProd(CPs[2], CPs[3]) +
       ellProd(CPs[3], CPs[4])) /
          5.0 +
      (2 * (ellProd(CPs[1], CPs[1]) + ellProd(CPs[3], CPs[3])) +
       ellProd(CPs[0], CPs[2]) + ellProd(CPs[2], CPs[4])) /
          15.0;

  // Penalty from 0.0 to 1.0
  P0 = P1 = P2 = synfig::Point3();
  for (k = 0, old = a, curr = a + 1; k < middle; ++k, old = curr, curr += 2) 
  {
    B = (middleAddedSequence[curr] - middleAddedSequence[old]) *
        (1 / (pars[k + 1] - pars[k]));
    A = middleAddedSequence[old] - B * pars[k];

    // Mixed integral
    P0 += int_B0a(A, B, pars[k], pars[k + 1]);
    P1 += int_B1a(A, B, pars[k], pars[k + 1]);
    P2 += int_B2a(A, B, pars[k], pars[k + 1]);

    // Sequence integral
    p += int_a2(A, B, pars[k], pars[k + 1]);
  }

  if (curr == b + 1) curr = b;
  B = (middleAddedSequence[curr] - middleAddedSequence[old]) *
      (1 / (pars[k + 1] - pars[k]));
  A = middleAddedSequence[old] - B * pars[k];

  // Mixed integral
  P0 += int_B0a(A, B, pars[k], 1.0);
  P1 += int_B1a(A, B, pars[k], 1.0);
  P2 += int_B2a(A, B, pars[k], 1.0);

  // Sequence integral
  p += int_a2(A, B, pars[k], 1.0);

  p -= 2 * (ellProd(P0, CPs[0]) + ellProd(P1, CPs[1]) + ellProd(P2, CPs[2]));

  // Penalty from 1.0 to 2.0
  P0 = P1 = P2 = synfig::Point3();
  for (k = pars.size() - 1, old = b, curr = b - 1; k > middle + 1; --k, old = curr, curr -= 2) 
  {
    B = (middleAddedSequence[curr] - middleAddedSequence[old]) *
        (1 / (pars[k] - pars[k - 1]));
    A = middleAddedSequence[curr] - B * (2 - pars[k - 1]);

    // Mixed integral
    P0 += int_B0a(A, B, 2 - pars[k], 2 - pars[k - 1]);
    P1 += int_B1a(A, B, 2 - pars[k], 2 - pars[k - 1]);
    P2 += int_B2a(A, B, 2 - pars[k], 2 - pars[k - 1]);

    // Sequence integral
    p += int_a2(A, B, 2 - pars[k], 2 - pars[k - 1]);
  }
  
  if (old == a + 1) curr = a;
  B = (middleAddedSequence[curr] - middleAddedSequence[old]) *
      (1 / (pars[k] - pars[k - 1]));
  A = middleAddedSequence[curr] - B * (2 - pars[k - 1]);

  // Mixed integral
  P0 += int_B0a(A, B, 2 - pars[k], 1.0);
  P1 += int_B1a(A, B, 2 - pars[k], 1.0);
  P2 += int_B2a(A, B, 2 - pars[k], 1.0);

  // Sequence integral
  p += int_a2(A, B, 2 - pars[k], 1.0);

  p -= 2 * (ellProd(P0, CPs[4]) + ellProd(P1, CPs[3]) + ellProd(P2, CPs[2]));

  // OCCHIO! Ho visto ancora qualche p<0! Da rivedere - non dovrebbe...
  if (p > p_max || p < 0)
    return 0;
  else
    len.l = p;

  return 1;
}

//--------------------------------------------------------------------------
inline etl::handle<synfig::Layer> convert(const Sequence &s, double penalty) 
{
  SkeletonGraph *graph = s.m_graphHolder;

  etl::handle<synfig::Layer> result;

  // First, we simplify the skeleton sequences found
  std::vector<unsigned int> reducedIndices;

  // NOTE: If s is circular, we have to protect head==tail 's adjacent nodes.
  // We then move away s tail and head, and insert them in the reducedIndices
  // apart from simplification.
  if (s.m_head == s.m_tail && graph->getNode(s.m_head).degree() == 2) 
  {
    Sequence t = s;

    SequenceSimplifier simplifier(&t);
    reducedIndices.push_back(s.m_head);

    t.m_head     = graph->getNode(s.m_head).getLink(0).getNext();
    t.m_headLink = !graph->getNode(t.m_head).linkOfNode(s.m_head);
    t.m_tail     = graph->getNode(s.m_tail).getLink(1).getNext();
    t.m_tailLink = !graph->getNode(t.m_tail).linkOfNode(s.m_tail);

    simplifier.simplify(reducedIndices);
    reducedIndices.push_back(s.m_tail);
  } 
  else 
  {
    SequenceSimplifier simplifier(&s);
    simplifier.simplify(reducedIndices);
  }

  // For segments, apply this immediate conversion
  if (reducedIndices.size() == 2) 
  {
    std::vector<synfig::Point3> segment(3);
    segment[0] = *graph->getNode(s.m_head);
    segment[1] = (*graph->getNode(s.m_head) + *graph->getNode(s.m_tail)) * 0.5;
    segment[2] = *graph->getNode(s.m_tail);
    
    return BezierToOutline(segment);
  }
  // when calculating sequence with 3 thick points where x,y are coordinates and z is thickness of stroke
  // it then build quadratic chunk using the three control points

  // Then, we convert the sequence in a quadratic stroke
  SequenceConverter converter(&s, penalty);
  result = converter(&reducedIndices);

  return result;
}

// Converts each forward or single Sequence of the image in its corresponding
// Stroke. 
// In synfig we will be using outline layer instead of TStroke  

void studio::conversionToStrokes(std::vector< etl::handle<synfig::Layer> > &strokes, VectorizerCoreGlobals &g,const etl::handle<synfig::Layer_Bitmap> &image) 
{
  SequenceList &singleSequences           = g.singleSequences;
  JointSequenceGraphList &organizedGraphs = g.organizedGraphs;
  double penalty                          = g.currConfig->m_penalty;
  max_thickness_zero                      = !g.currConfig->m_maxThickness; // if any value then false otherwise 0 then true
  unsigned int i, j, k;
  topleft = image->param_tl.get(synfig::Point());
  bottomright = image->param_br.get(synfig::Point());
  canvas = image->get_canvas();
  // Convert single sequences
  for (i = 0; i < singleSequences.size(); ++i) 
  {
    if (singleSequences[i].m_head == singleSequences[i].m_tail) 
    {
      // If the sequence is circular, move your endpoints to an edge middle, in
      // order
      // to allow a soft junction
      SkeletonGraph *currGraph = singleSequences[i].m_graphHolder;

      unsigned int head     = singleSequences[i].m_head;
      unsigned int headLink = singleSequences[i].m_headLink;
      unsigned int next = currGraph->getNode(head).getLink(headLink).getNext();
      unsigned int nextLink = currGraph->getNode(next).linkOfNode(head);

      unsigned int addedNode = singleSequences[i].m_graphHolder->newNode(
          (*currGraph->getNode(head) + *currGraph->getNode(next)) * 0.5);

      singleSequences[i].m_graphHolder->insert(addedNode, head, headLink);
      *singleSequences[i].m_graphHolder->node(addedNode).link(0) =
          *singleSequences[i].m_graphHolder->node(head).link(headLink);

      singleSequences[i].m_graphHolder->insert(addedNode, next, nextLink);
      *singleSequences[i].m_graphHolder->node(addedNode).link(1) =
          *singleSequences[i].m_graphHolder->node(next).link(nextLink);

      singleSequences[i].m_head     = addedNode;
      singleSequences[i].m_headLink = 0;
      singleSequences[i].m_tail     = addedNode;
      singleSequences[i].m_tailLink = 1;
    }

    strokes.push_back(convert(singleSequences[i], penalty));
  }

  // Convert graph sequences
  for (i = 0; i < organizedGraphs.size(); ++i)
    for (j = 0; j < organizedGraphs[i].getNodesCount(); ++j)
      if (!organizedGraphs[i].getNode(j).hasAttribute(
              JointSequenceGraph::ELIMINATED))
        // Otherwise eliminated by junction recovery
        for (k = 0; k < organizedGraphs[i].getNode(j).getLinksCount(); ++k) {
          // A sequence is taken at both extremities in our organized graphs
          if (organizedGraphs[i].getNode(j).getLink(k)->isForward())
            strokes.push_back(
                convert(*organizedGraphs[i].getNode(j).getLink(k), penalty));
        }
}

  
/*!
\ingroup PkgHDSConcepts
\cgalconcept

The concept `HalfedgeDSHalfedge` defines the requirements for the local `Halfedge` 
type in the `HalfedgeDS` concept. It is also required in 
the `Halfedge_wrapper<Refs,Traits>` member class template of an 
items class, see the `HalfedgeDSItems` concept. 

A halfedge is an oriented edge between two vertices. It is always 
paired with a halfedge pointing in the opposite direction. The 
`opposite()` member function returns this halfedge of opposite 
orientation. The `next()` member function points to the successor 
halfedge along the face - or if the halfedge is a border halfedge - 
along the open region. A halfedge optionally stores a reference to the 
previous halfedge along the face, a reference to an incident vertex, 
and a reference to an incident face. Type tags indicate whether the 
related member functions are supported. 
Figure \ref figureHalfedgeDSOptionalMethods 
depicts the relationship between a halfedge and its incident 
halfedges, vertices, and faces. 

\anchor figureHalfedgeDSOptionalMethods 
\image html hds_optional.gif "The three classes `Vertex`, `Halfedge`, and `Face` of the halfedge data structure. Member functions with shaded background are mandatory. The others are optionally supported."  

For the protection of the integrity of the data structure classes such 
as `CGAL::Polyhedron_3` are allowed to redefine the modifying member 
functions to be private. In order to make them accessible for the 
halfedge data structure they must be derived from a base class 
`Base` where the modifying member functions are still public. Even 
more protection is provided for the `set_opposite()` member 
function. The base class `Base_base` provides access to it. (The 
protection could be bypassed also by an user, but not by accident.) 

\hasModel ::CGAL::HalfedgeDS_halfedge_base
\hasModel ::CGAL::HalfedgeDS_halfedge_min_base

\sa `HalfedgeDS<Traits,Items,Alloc>` 
\sa `HalfedgeDSItems` 
\sa `HalfedgeDSVertex` 
\sa `HalfedgeDSFace` 

*/

class HalfedgeDSHalfedge {
public:

/// \name Types 
/// @{

/*! 
instantiated `HalfedgeDS` ( \f$ \equiv\f$ `Refs`). 
*/ 
typedef Hidden_type HalfedgeDS; 

/*! 
base class that allows modifications. 
*/ 
typedef Hidden_type Base; 

/*! 
base class to access `set_opposite()`. 
*/ 
typedef Hidden_type Base_base; 

/*! 
model of `HalfedgeDSVertex`. 
*/ 
typedef Hidden_type Vertex; 

/*! 
model of `HalfedgeDSFace`. 
*/ 
typedef Hidden_type Face; 

/*! 
handle to vertex. 
*/ 
typedef Hidden_type Vertex_handle; 

/*! 
handle to halfedge. 
*/ 
typedef Hidden_type Halfedge_handle; 

/*! 
handle to face. 
*/ 
typedef Hidden_type Face_handle; 

/*! 

*/ 
typedef Hidden_type Vertex_const_handle; 

/*! 

*/ 
typedef Hidden_type Halfedge_const_handle; 

/*! 

*/ 
typedef Hidden_type Face_const_handle; 

/*! 
`CGAL::Tag_true` or 
`CGAL::Tag_false`. 
*/ 
typedef Hidden_type Supports_halfedge_prev; 

/*! 
~ 
*/ 
typedef Hidden_type Supports_halfedge_vertex; 

/*! 
~ 
*/ 
typedef Hidden_type Supports_halfedge_face; 

/// @} 

/// \name Creation 
/// @{

/*! 
default constructor. 
*/ 
Halfedge(); 

/// @} 

/// \name Operations 
/// @{

/*! 

*/ 
Halfedge_handle opposite(); 

/*! 
the opposite halfedge. 
*/ 
Halfedge_const_handle opposite() const; 

/*! 

sets opposite halfedge to `h`. 
*/ 
void set_opposite( Halfedge_handle h); 

/*! 

*/ 
Halfedge_handle next(); 

/*! 
the next halfedge around the face. 
*/ 
Halfedge_const_handle next() const; 

/*! 

sets next halfedge to `h`. 
*/ 
void set_next( Halfedge_handle h); 

/*! 
is true if `h` is a border halfedge. 
*/ 
bool is_border() const; 

/// @} 

/// \name Operations available if `Supports_halfedge_prev` \f$ \equiv\f$ `CGAL::Tag_true` 
/// @{

/*! 

*/ 
Halfedge_handle prev(); 

/*! 
the previous halfedge around the face. 
*/ 
Halfedge_const_handle prev() const; 
/*! 

sets prev halfedge to `h`. 
*/ 
void set_prev( Halfedge_handle h); 

/// @} 

/// \name Operations available if `Supports_halfedge_vertex` \f$ \equiv\f$  `CGAL::Tag_true` 
/// @{

/*! 

*/ 
Vertex_handle vertex(); 

/*! 
the incident vertex of `h`. 
*/ 
Vertex_const_handle vertex() const; 
/*! 

sets incident vertex to `v`. 
*/ 
void set_vertex( Vertex_handle v); 

/// @} 

/// \name Operations available if `Supports_halfedge_face` \f$ \equiv\f$ 
/// @{

/*! 

*/ 
Face_handle face(); 

/*! 
the incident face of `h`. If `h` is a border halfedge 
the result is default construction of the handle. 
*/ 
Face_const_handle face() const; 

/*! 

sets incident face to `f`. 
*/ 
void set_face( Face_handle f); 

/// @}

}; /* end HalfedgeDSHalfedge */

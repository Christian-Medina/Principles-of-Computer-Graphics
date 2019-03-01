///////////////////////////////////////////////////////////////////////////////
// gfxmath.hh
//
// Linear algebra for graphics. This header includes two classes,
// gfx::vector that represents a low-dimensional vector, and
// gfx::matrix that represents a low-dimensional matrix. These classes
// are intended specifically for computer graphics applications so
// make some simplifying assumptions:
//
// - The dimensions of each vector/matrix is known at compile-time,
//   and is expressed as a template parameter.
//
// - Numerical: the elements of a vector/matrix may only be numeric
//   types, most likely int, float, or double. These classes do not
//   support vectors/matrices whose elements are non-numeric, such as
//   a vector of strings or matrix of matrices, even though those
//   concepts may be well-defined mathematically. In particular, this
//   code assumes that an element may be passed and returned by value,
//   and initialized to literal constants 0 and 1.
//
// - Low-dimension: vectors/matrices are only expected to be 4D or
//   smaller, so the largest anticipated data structure is a 4x4
//   matrix of doubles, occupying only 64 bytes. Therefore it is
//   reasonable to pass and return vectors/matrices by value, and
//   store them in stack-allocated arrays.
//
// - Further, since the dimension is small, Cramer's rule is an
//   acceptable algorithm for solving linear systems.
//
// Coding conventions:
//
// - All this code resides inside the gfx:: namespace, short for
//   "graphics".
//
// - Template parameters use the same order as std::array. So a vector
//   is declared as gfx::vector<scalar_type, dimension> and a matrix
//   is declared as gfx::matrix<scalar_type, height, width>.
//
// - "Vector" is the proper name for a mathematical vector, so we are
//   using it, even though it clashes with std::vector. Use
//   std::vector for teh STL data structure and gfx::vector for the
//   linear-algebra vector type.
//
// - Using-declarations are used to create concise aliases for long
//   types. There are aliases for vector2, vector3, vector4, and
//   matrix2x2, matrix3x3, and matrix4x4.
//
// - Since vector/matrix dimensions are template parameters, we use
//   types aggressively to ensure that math expressions are
//   well-typed, and fail early at compile-time when they are not. For
//   example the matrix * operator insists that the matrix operands
//   have compatible dimensions, so incompatible multiplies trigger
//   compile errors.
//
// - In operator overloads, "lhs" refers to the left-hand-side
//   operand, and "rhs" refers to the right-hand-side operand.
//
// - Function preconditions are checked with assertions. Compile-time
//   assertions with static_assert() are preferred over run-time
//   assertions with assert(), to catch errors as early as possible.
//
// - Default constructors initalize all vector/matrix elements to zero.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <initializer_list>
#include <memory>
#include <ostream>

namespace gfx {

  const double DEFAULT_DELTA = .001;

  // Return true when lhs and rhs are approximately equal. This
  // function is intended to compare floating points values for
  // equality, while ignoring small rounding errors. delta is the
  // maximum error, as a fraction of rhs, that counts as equal. delta
  // must be positive. Ordinarily you want delta to be a small postive
  // fraction, such as .001. This function handles positive and
  // negative infinity properly.
  template <typename scalar_type>
  bool almost_equal(scalar_type lhs,
		    scalar_type rhs,
		    double delta = DEFAULT_DELTA) {
    assert(delta > 0.0);
    if (lhs == rhs) {
      // Handle values considered truly equal according to ==, as well
      // as infinite values.
      return true;
    } else {
      // Use the delta calculation, using absolute values to be
      // careful about negative values.
      double difference = static_cast<double>(lhs) - static_cast<double>(rhs),
	positive_ratio = std::abs(difference / rhs);
      return (positive_ratio <= delta);
    }
  }

  // Class for a mathematical vector with DIMENSION elements, each of
  // type scalar_type.
  template <typename scalar_type, int DIMENSION>
  class vector {
  public:

    using same_type = gfx::vector<scalar_type, DIMENSION>;

    static bool is_index(int i) {
      return ((i >= 0) && (i < DIMENSION));
    }
    
    vector(scalar_type default_value = 0) {
      fill(default_value);
    }

    // Initializer list constructor. You can use this with something like
    //
    // gfx::vector<double, 3> v({1.0, 2.0, 3.0});
    //
    // to initialize v with the values <1, 2, 3>.
    //
    // If you provide fewer values than the vector contains, the
    // unmentioned elements are initialized to zero. If you provide
    // more values, the extras are ignored.
    vector(std::initializer_list<scalar_type> il) {
      auto it = il.begin();
      for (int i = 0; i < DIMENSION; ++i) {
	if (it != il.end()) {
	  _elements[i] = *it;
	  it++;
	} else {
	  _elements[i] = 0;
	}
      }
    }

    vector(const same_type& init)
      : _elements(init._elements) { }

    same_type& operator=(const same_type& rhs) {
      _elements = rhs._elements;
      return *this;
    }

    bool operator==(const same_type& rhs) const {
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    if ((*this)[i] != rhs[i])
			    return false;
	    }
	    return true;
    }

    bool operator!=(const same_type& rhs) const {
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    if ((*this)[i] != rhs[i])
			    return true;
	    }
	    return false;
    }


    friend std::ostream& operator<<(std::ostream& os, const same_type& rhs) {
	    os << "<";
      for (int i = 0; i < DIMENSION; ++i) {
	if (i > 0) {
	  os << ", ";
	}
	os << rhs._elements[i];
      }
      os << ">";
      return os;
    }

    const scalar_type& operator[](int i) const {
      assert(is_index(i));
      return _elements[i];
    }
    
    scalar_type& operator[](int i) {
      assert(is_index(i));
      return _elements[i];
    }

    // Add a vector.
    same_type operator+(const same_type& rhs) const {
	    same_type temp((*this).dimension());
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    temp[i] = (*this)[i] + rhs[i];
	    }
      return temp;
    }

    // Subtract a vector.
    same_type operator-(const same_type& rhs) const {
	    same_type temp((*this).dimension());
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    temp[i] = (*this)[i] - rhs[i];
	    }
      return temp;
    }

    // Negate this vector.
    same_type operator-() const {
	    same_type temp((*this).dimension());
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    temp[i] = -(*this)[i];
	    }
      return temp;
    }

    // Multiply by a scalar.
    same_type operator*(scalar_type rhs) const {
	    same_type temp((*this).dimension());
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    temp[i] = (rhs*(*this)[i]);
	    }
      return temp;
    }

    // Multiply by a vector (dot product). Cross product is
    // implemented in the cross() function below.
    scalar_type operator*(const same_type& rhs) const {
	    scalar_type temp = 0;
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    temp += ((*this)[i] * rhs[i]);
	    }
      return temp;
    }

    // Divide by a scalar.
    same_type operator/(scalar_type rhs) const {
	    same_type temp = 0;
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    temp[i] = ((*this)[i] / rhs);
	    }
      return temp;
    }

    // Return true when this vector is approximately equal to rhs,
    // according to the almost_equal function.
    bool almost_equal(const same_type& rhs, double delta = DEFAULT_DELTA) const {
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    if (rhs[i] >= ((*this)[i] + delta) || rhs[i] <= ((*this)[i] - delta))
			    return false;
	    }

	    return true;
    }

    int dimension() const {
	    return DIMENSION;
    }

    // Set all elements to value.
    void fill(scalar_type value) {
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    (*this)[i] = value;
	    }
	    return;
    }

    // The magnitude of this vector, squared (i.e. raised to the
    // second power). This function is provided because it should be
    // faster than magnitude(), and in some use cases
    // magnitude_squared() is sufficient.
    scalar_type magnitude_squared() const {
	    scalar_type temp = 0;
	    for (int i = 0; i < (*this).dimension(); i++)
	    {
		    temp += pow((*this)[i],2);
	    }
      return temp;
    }

    // The magnitude of this vector, i.e. the square root of the sum
    // of the squares of the elements.
    scalar_type magnitude() const {
	    scalar_type temp = magnitude_squared();
	    temp = pow(temp,0.5);
      return temp;
    }

    // Return a vector with the same direction as this vector, but
    // whose magnitude is normalized to 1.
    same_type normalize() const {
	    scalar_type mag = magnitude();
	    same_type temp;
	    for (int i = 0; i < (*this).dimension(); i++)
	    {
		    temp[i] = ((*this)[i] / mag);
	    }
      return temp;
    }

    // Return the cross product this x rhs . Note, since in general
    // cross product is not commutative, that this matrix object is
    // the left operand. This operation is only defined for 3D
    // vectors.
    same_type cross(const same_type& rhs) const {
	    same_type temp((*this).dimension());
	    for(int i = 0; i < (*this).dimension(); i++)
	    {
		    if (i == 0)
		    {
			    temp[i] = ((*this)[((i+1)%3)] * rhs[2]) - (rhs[((i+1)%3)] * (*this)[2]);
		    }
		    else{
			    temp[i] = ((*this)[((i+1)%3)] * rhs[((i-1)%3)]) - (rhs[((i+1)%3)] * (*this)[((i-1)%3)]);
		    }
	    }
      return temp;
    }

    // Return a portion of this vector, specifically RESULT_DIMENSION
    // elements starting from start_index. The result dimension cannot
    // be larger than this source vector, and the range of indices in
    // the subvector must actually exist in this vector.
    template<const int RESULT_DIMENSION>
    gfx::vector<scalar_type, RESULT_DIMENSION> subvector(int start_index) const {

      static_assert(RESULT_DIMENSION <= DIMENSION,
		    "subvector cannot be larger than original vector");

      assert(is_index(start_index + RESULT_DIMENSION - 1));
      gfx::vector<scalar_type, RESULT_DIMENSION> temp;
     for (int i = 0; i < RESULT_DIMENSION; i++)
     {
	temp[i] = (*this)[start_index + i];
     } 
      return temp;
    }

    // Return a lower-dimension copy of this vector, which includes
    // only the first RESULT_DIMENSION elements.
    template<int RESULT_DIMENSION>
    gfx::vector<scalar_type, RESULT_DIMENSION> shrink() const {
      static_assert(RESULT_DIMENSION < DIMENSION,
		    "shrink'd vector must be smaller");
      gfx::vector<scalar_type, RESULT_DIMENSION> temp;
     for (int i = 0; i < RESULT_DIMENSION; i++)
     {
	temp[i] = (*this)[i];
     } 
      return temp;
    }

    // Return a higher-dimension copy of this vector. The
    // newly-created elements are all initialized to
    // default_value. The new dimension must be strictly greater than
    // this vector's dimension.
    template<int RESULT_DIMENSION>
    gfx::vector<scalar_type, RESULT_DIMENSION> grow(scalar_type default_value = 0) {
      
      static_assert(RESULT_DIMENSION > DIMENSION,
		    "grown vector must be larger than original vector");

      gfx::vector<scalar_type, RESULT_DIMENSION> temp;
     for (int i = 0; i < DIMENSION; i++)
     {
	temp[i] = (*this)[i];
     } 
     for (int i = DIMENSION; i < RESULT_DIMENSION; i++)
     {
	temp[i] = default_value;
     }
      return temp;
    }

  private:
    
    std::array<scalar_type, DIMENSION> _elements;
  };

  // Aliases for vectors of 2, 3, and 4 dimensions.
  template <typename scalar_type> using vector2 = gfx::vector<scalar_type, 2>;
  template <typename scalar_type> using vector3 = gfx::vector<scalar_type, 3>;
  template <typename scalar_type> using vector4 = gfx::vector<scalar_type, 4>;

  // Class for a mathematical matrix with HEIGHT rows, WIDTH columns,
  // and each element is of type scalar_type. Each row is a
  // gfx::vector object.
  template <typename scalar_type,
	    int HEIGHT,
	    int WIDTH>
  class matrix {
  public:

    using same_type = gfx::matrix<scalar_type, HEIGHT, WIDTH>;
    using row_type = gfx::vector<scalar_type, WIDTH>;

    static constexpr bool is_row(int r) {
      return ((r >= 0) && (r < HEIGHT));
    }

    static constexpr bool is_column(int c) {
      return ((c >= 0) && (c < WIDTH));
    }

    // Return true when this matrix is square, i.e. its width and
    // height are identical.
    static constexpr bool is_square() {
      return (WIDTH == HEIGHT);
    }

    matrix(scalar_type default_value = 0) {
      fill(default_value);
    }

    matrix(const same_type& rhs)
      : _rows(rhs._rows) { }

    // Initializer list constructor. Elements are added in row-major
    // order, i.e. the first row is filled left-to-right, then the
    // second row, and so on. You can use this with something like
    //
    // gfx::matrix<double, 2, 2> m({1.0, 2.0, 3.0, 4.0});
    //
    // to initialize m with the values
    //    |1 2|
    //    |3 4| .
    //
    // If you provide fewer values than the matrix contains, the
    // unmentioned elements are initialized to zero. If you provide
    // more values, the extras are ignored.
    matrix(std::initializer_list<scalar_type> il) {
      auto it = il.begin();
      for (int i = 0; i < HEIGHT; ++i) {
	for (int j = 0; j < WIDTH; ++j) {
	  if (it != il.end()) {
	    _rows[i][j] = *it;
	    it++;
	  } else {
	    _rows[i][j] = 0;
	  }
	}
      }
    }

    same_type& operator=(const same_type& rhs) {
      _rows = rhs._rows;
      return *this;
    }

    bool operator==(const same_type& rhs) const {
	    for(int i = 0; i < HEIGHT; i++)
	    {
		    for(int j = 0; j < WIDTH; j++)
		    {
			    if((*this)[i][j] != rhs[i][j])
			    {
				    return false;
			    }
		    }
	    }
      return true;
    }

    bool operator!=(const same_type& rhs) const {
	    for(int i = 0; i < HEIGHT; i++)
	    {
		    for(int j = 0; j < WIDTH; j++)
		    {
			    if((*this)[i][j] != rhs[i][j])
			    {
				    return true;
			    }
		    }
	    }
      return false;
    }

    friend std::ostream& operator<<(std::ostream& os, const same_type& rhs) {
      for (int i = 0; i < HEIGHT; ++i) {
	os << "|";
	for (int j = 0; j < WIDTH; ++j) {
	  if (j > 0) {
	    os << " ";
	  }
	  os << rhs._rows[i][j];
	}
	os << "|" << std::endl;
      }
      return os;
    }

    const row_type& operator[](int row) const {
      assert(is_row(row));
      return _rows[row];
    }
    
    row_type& operator[](int row) {
      assert(is_row(row));
      return _rows[row];
    }

    // Add a matrix.
    same_type operator+(const same_type& rhs) const {
	    same_type temp;
	    for(int i = 0; i < HEIGHT; i++)
	    {
		    for (int j = 0; j < WIDTH; j++)
		    {
			    temp[i][j] = ((*this)[i][j] + rhs[i][j]);
		    }
	    }
      return temp;
    }

    // Subtract a matrix.
    same_type operator-(const same_type& rhs) const {
	    same_type temp;
	    for(int i = 0; i < HEIGHT; i++)
	    {
		    for (int j = 0; j < WIDTH; j++)
		    {
			    temp[i][j] = ((*this)[i][j] - rhs[i][j]);
		    }
	    }
      return temp;
    }

    // Negate this matrix.
    same_type operator-() const {
	    same_type temp;
	    for(int i = 0; i < HEIGHT; i++)
	    {
		    for(int j = 0; j < WIDTH; j++)
		    {
			    temp[i][j] = -(*this)[i][j];
		    }
	    }
      return temp;
    }

    // Divide by a scalar.
    same_type operator/(scalar_type rhs) const {
	    same_type temp;
	    for(int i = 0; i < HEIGHT; i++)
	    {
		    for(int j = 0; j < WIDTH; j++)
		    {
			    temp[i][j] = ((*this)[i][j])/rhs;
		    }
	    }
      return temp;
    }

    // Multiply by a scalar.
    same_type operator*(scalar_type rhs) const {
	    same_type temp;
	    for(int i = 0; i < HEIGHT; i++)
	    {
		    for(int j = 0; j < WIDTH; j++)
		    {
			    temp[i][j] = ((*this)[i][j])*rhs;
		    }
	    }
      return temp;
    }

    // Multiply by a matrix.
    template<int RESULT_WIDTH>
	    gfx::matrix<scalar_type, HEIGHT, RESULT_WIDTH>
	    operator*(const gfx::matrix<scalar_type, WIDTH, RESULT_WIDTH>& rhs) const {
		    gfx::matrix<scalar_type, HEIGHT, RESULT_WIDTH> temp_m;
		    scalar_type temp_s = 0;
		    for(int i = 0; i < HEIGHT; i++)
		    {
			    for(int j = 0; j < RESULT_WIDTH; j++)
			    {
				    temp_s = 0;
				    for(int d = 0; d < WIDTH; d++)
				    {
					    temp_s += ((*this)[i][d] * rhs[d][j]);
				    }
				    temp_m[i][j] = temp_s;
			    }
		    }

		    return temp_m;
	    } 
    // Return a portion of this matrix, of height RESULT_HEIGHT and
    // width RESULT_WIDTH, starting from the "top" row and "left"
    // column. The submatrix must actually fit inside this matrix.
    template<int RESULT_HEIGHT, int RESULT_WIDTH>
	    gfx::matrix<scalar_type, RESULT_HEIGHT, RESULT_WIDTH> submatrix(int top,
			    int left) const {
      static_assert(RESULT_HEIGHT > 0,
		    "submatrix height must be positive");
      static_assert(RESULT_WIDTH > 0,
		    "submatrix width must be positive");
      static_assert(RESULT_HEIGHT <= HEIGHT,
		    "submatrix height must be less than source matrix height");
      static_assert(RESULT_WIDTH <= WIDTH,
		    "submatrix width must be less than source matrix width");
      gfx::matrix<scalar_type, RESULT_HEIGHT, RESULT_WIDTH> temp;
      for(int i = top; i < (RESULT_HEIGHT + top); i++)
      {
	      for(int j = left; j < (RESULT_WIDTH + left); j++)
	      {
		      temp[i-top][j-left] = (*this)[i][j];
	      }
      }
      
      return temp;
    }

    // Return a smaller version of this matrix, keeping only the first
    // RESULT_HEIGHT rows and RESULT_WIDTH columns. The resulting
    // matrix must be strictly smaller than this matrix.
    template<int RESULT_HEIGHT, int RESULT_WIDTH>
    gfx::matrix<scalar_type, RESULT_HEIGHT, RESULT_WIDTH> shrink() const {
      static_assert(RESULT_HEIGHT <= HEIGHT,
		    "shrunk matrix height must be less than or equal to source matrix height");
      static_assert(RESULT_WIDTH <= WIDTH,
		    "shrunk matrix width must be less than or equal to source matrix width");
      static_assert((RESULT_WIDTH < WIDTH) || (RESULT_HEIGHT < HEIGHT),
		    "shrunk matrix must be strictly smaller than the source matrix");
      gfx::matrix<scalar_type, RESULT_HEIGHT, RESULT_WIDTH> temp;
      for(int i = 0; i < (RESULT_HEIGHT); i++)
      {
	      for(int j = 0; j < (RESULT_WIDTH); j++)
	      {
		      temp[i][j] = (*this)[i][j];
	      }
      }
      
      return temp;
    }

    // Return a larger version of this matrix. The newly-created
    // elements are all initialized to default_value. The new vector
    // must be larger than this vector.
    template<int RESULT_HEIGHT, int RESULT_WIDTH>
    gfx::matrix<scalar_type, RESULT_HEIGHT, RESULT_WIDTH> grow(scalar_type default_value = 0) const {
      static_assert(RESULT_HEIGHT >= HEIGHT,
		    "enlarged matrix must be higher than source matrix");
      static_assert(RESULT_WIDTH >= WIDTH,
		    "enlarged matrix must be wider than source matrix");
      static_assert((RESULT_HEIGHT > HEIGHT) || (RESULT_WIDTH > WIDTH),
		    "enlarged matrix must be larger than source matrix");
   gfx::matrix<scalar_type, RESULT_HEIGHT, RESULT_WIDTH> temp;
      
      for(int i = 0; i < RESULT_HEIGHT; i++)
      {
	      for(int j = 0; j < RESULT_WIDTH; j++)
	      {
		      temp[i][j] = default_value;
	      }
      }
      for(int i = 0; i < (HEIGHT); i++)
      {
	      for(int j = 0; j < (WIDTH); j++)
	      {
		      temp[i][j] = (*this)[i][j];
	      }
      }

      return temp;
    }
    
    // Return true when this matrix is approximately equal to rhs,
    // according to the almost_equal function.
    bool almost_equal(const same_type& rhs,
		      double delta = DEFAULT_DELTA) const {
      for (int i = 0; i < HEIGHT; ++i) {
	if (!_rows[i].almost_equal(rhs[i], delta)) {
	  return false;
	}
      }
      return true;
    }

    // Return one column of this matrix as a matrix object.
    gfx::matrix<scalar_type, HEIGHT, 1> column_matrix(int column) const {
	    gfx::matrix<scalar_type, HEIGHT, 1> temp;
	    for (int i = 0; i < HEIGHT; i++)
	    {
		    temp[i][0] = (*this)[i][column];
	    }
      return temp;
    }

    // Return one column of this matrix as a vector object.
    gfx::vector<scalar_type, HEIGHT> column_vector(int column) const {
      gfx::vector<scalar_type, HEIGHT> temp;
	    for (int i = 0; i < HEIGHT; i++)
	    {
		    temp[i] = (*this)[i][column];
	    }
      return temp;
    }

    // Assign all elements in this matrix to value.
    void fill(scalar_type value) {
	    for (int i = 0; i < HEIGHT; i++)
	    {
		    for (int j = 0; j < WIDTH; j++)
		    {
			   (*this)[i][j] = value;
		    }
	    } 
    }

    // Return the height of this matrix.
    static int height() {
      return HEIGHT;
    }

    // Return an identity matrix with the same dimensions as this
    // matrix, which must be square.
    static same_type identity() {
      static_assert(is_square(),
		    "identity matrix must be square");		    
     	same_type temp;
	for(int i = 0; i < HEIGHT; i++)
	{
		for(int j = 0; j < HEIGHT; j++)
		{
			if(i == j)
			{
				temp[i][j] = 1;
			}
			else
			{
				temp[i][j] = 0;
			}
		}
	}
      return temp;
    }

    // Return one row of this matrix as a matrix object.
    gfx::matrix<scalar_type, 1, WIDTH> row_matrix(int row) const {
      assert(is_row(row));
      gfx::matrix<scalar_type, 1, WIDTH> temp;
	    for (int i = 0; i < WIDTH; i++)
	    {
		    temp[0][i] = (*this)[row][i];
	    }
      return temp;
    }

    // Return one row of this matrix as a vector object.
    row_type row_vector(int row) const {
	    row_type temp;
	    for (int i = 0; i < WIDTH; i++)
	    {
		    temp[i] = (*this)[row][i];
	    }
	    return temp;
    }

    // Return the transposition of this matrix.
    gfx::matrix<scalar_type, WIDTH, HEIGHT> transpose() const {
	    gfx::matrix<scalar_type, HEIGHT, WIDTH> temp;
	    for(int i = 0; i < HEIGHT; i ++)
	    {
		    for(int j = 0; j < WIDTH; j++)
		    {
			    temp[i][j] = (*this)[j][i];
		    }
	    }
      return temp;
    }

    // Return the width of this matrix.
    static int width() {
      return WIDTH;
    }

    // Return the determinant of this matrix. This function is only
    // implemented for square 2x2 and 3x3 matrices.
    scalar_type determinant() const {
      static_assert(is_square(),
		    "determinant is only defined for square matrices");
      static_assert((WIDTH == 2) || (WIDTH == 3),
		    "determinant only implemented for 2x2 and 3x3 matrices");
	scalar_type temp = 0;
	if(WIDTH == 2)
	{
		temp += ((*this)[0][0] * (*this)[1][1]) - ((*this)[1][0] * (*this)[0][1]);
	}
	
	if(WIDTH == 3)
	{
		scalar_type a,b,c,d,e,f,g,h,i;
		a = (*this)[0][0];
		b = (*this)[0][1];
		c = (*this)[0][2];
		d = (*this)[1][0];
		e = (*this)[1][1];
		f = (*this)[1][2];
		g = (*this)[2][0];
		h = (*this)[2][1];
		i = (*this)[2][2];

		temp += (a*e*i) + (b*f*g) + (c*d*h) - (c*e*g) - (b*d*i) - (a*f*h);
	}
      return temp;
    }

    // Solve a linear system, Ax=b, where this matrix contains the
    // coefficients A, and the passed-in vector b contains the
    // constants on the right-hand-side of the equations. This
    // function is only implemented for square 2x2 and 3x3
    // matrices. This function uses Cramer's rule, which is reasonably
    // efficient for matrices up to 4x4, but would be slow for large
    // matrices.
    gfx::vector<scalar_type, HEIGHT>
    solve(const gfx::vector<scalar_type, HEIGHT>& b) const {
      static_assert(is_square(),
		    "only square linear systems can be solved");
      static_assert((WIDTH == 2) || (WIDTH == 3),
		    "solve is only implemented for 2x2 and 3x3 matrices");
	if(HEIGHT == 2)
	{
		scalar_type determ = (*this).determinant();
		vector<scalar_type, HEIGHT> result;
		same_type temp = (*this);
		temp[0][0] = b[0];
		temp[1][0] = b[1];
		result[0] = temp.determinant()/determ;

		temp = (*this);
		temp[0][1] = b[0];
		temp[1][1] = b[1];
		result[1] = temp.determinant()/determ;
		return result;
	}
	if(HEIGHT == 3)
	{
		scalar_type determ = (*this).determinant();
		vector<scalar_type, HEIGHT> result;
		same_type temp = (*this);
		temp[0][0] = b[0];
		temp[1][0] = b[1];
		temp[2][0] = b[2];
		result[0] = temp.determinant()/determ;

		temp = (*this);
		temp[0][1] = b[0];
		temp[1][1] = b[1];
		temp[2][1] = b[2];
		result[1] = temp.determinant()/determ;

		temp = (*this);
		temp[0][2] = b[0];
		temp[1][2] = b[1];
		temp[2][2] = b[2];
		result[2] = temp.determinant()/determ;
		return result;
	}
	

      return gfx::vector<scalar_type, HEIGHT>();
    }    
    
  private:
    std::array<row_type, HEIGHT> _rows;
  };
    
  template <typename scalar_type> using matrix2x2 = gfx::matrix<scalar_type, 2, 2>;
  template <typename scalar_type> using matrix3x3 = gfx::matrix<scalar_type, 3, 3>;
  template <typename scalar_type> using matrix4x4 = gfx::matrix<scalar_type, 4, 4>;
}

///////////////////////////////////////////////////////////////////////////////
// gfxmath_test.cc
//
// Unit tests for gfxmath.hh
//
///////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <sstream>

#include "rubrictest.hh"

#include "gfxmath.hh"

template <int DIMENSION>
using int_vector = gfx::vector<int, DIMENSION>;
  
using test_vector = int_vector<3>;

template <int HEIGHT, int WIDTH>
using int_matrix = gfx::matrix<int, HEIGHT, WIDTH>;

using int_matrix_2x2 = int_matrix<2, 2>;
using int_matrix_3x3 = int_matrix<3, 3>;

int main() {
  
  Rubric r;

  const test_vector zeroes,
    threes(3),
    one_two_three({1, 2, 3}),
    copied(one_two_three);

  r.criterion("vector constructor",
	      1,
	      [&]() {
		TEST_EQUAL("default constructor dimension", 3, zeroes.dimension());
		TEST_EQUAL("default constructor element 0", 0, zeroes[0]);
		TEST_EQUAL("default constructor element 1", 0, zeroes[1]);
		TEST_EQUAL("default constructor element 2", 0, zeroes[2]);
		
		TEST_EQUAL("fill constructor dimension", 3, threes.dimension());
		TEST_EQUAL("fill constructor element 0", 3, threes[0]);
		TEST_EQUAL("fill constructor element 1", 3, threes[1]);
		TEST_EQUAL("fill constructor element 2", 3, threes[2]);

		TEST_EQUAL("initializer list constructor dimension", 3, one_two_three.dimension());
		TEST_EQUAL("initializer list constructor element 0", 1, one_two_three[0]);
		TEST_EQUAL("initializer list constructor element 1", 2, one_two_three[1]);
		TEST_EQUAL("initializer list constructor element 2", 3, one_two_three[2]);

		TEST_EQUAL("copy constructor dimension", 3, copied.dimension());
		TEST_EQUAL("copy constructor element 0", 1, copied[0]);
		TEST_EQUAL("copy constructor element 1", 2, copied[1]);
		TEST_EQUAL("copy constructor element 2", 3, copied[2]);
	      });

  r.criterion("vector operators",
	      2,
	      [&]() {
		// =
		{
		  auto assigned = threes;
		  TEST_EQUAL("=", threes, assigned);
		  assigned = one_two_three;
		  TEST_EQUAL("=", one_two_three, assigned);
		}
		
		// ==
		TEST_TRUE("== true", (zeroes == zeroes));
		TEST_TRUE("== true", (threes == threes));
		TEST_TRUE("== true", (one_two_three == one_two_three));
		TEST_TRUE("== true", (one_two_three == copied));
		TEST_FALSE("== false", (zeroes == threes));
		TEST_FALSE("== false", (threes == one_two_three));

		// !=
		TEST_TRUE("!= true", (zeroes != one_two_three));
		TEST_TRUE("!= true", (threes != one_two_three));
		TEST_FALSE("!= false", (threes != threes));
		TEST_FALSE("!= false", (one_two_three != copied));

		// <<
		std::stringstream ss;
		ss << one_two_three;
		TEST_EQUAL("<<", "<1, 2, 3>", ss.str());
		
		// const []
		TEST_EQUAL("const [0]", 1, one_two_three[0]);
		TEST_EQUAL("const [1]", 2, one_two_three[1]);
		TEST_EQUAL("const [2]", 3, one_two_three[2]);

		// non-const []
		{
		  auto mutable_copy(one_two_three);
		  mutable_copy[1] = 7;
		  TEST_EQUAL("non-const [1]", test_vector({1, 7, 3}), mutable_copy);
		}

		// +
		TEST_EQUAL("{3, 3, 3} + {1, 2, 3}", test_vector({4, 5, 6}), threes + one_two_three);
		TEST_EQUAL("{1, 2, 3} + {0, 0, 0}", one_two_three, one_two_three + zeroes);

		// - (binary operator)
		TEST_EQUAL("{3, 3, 3} - {1, 2, 3}", test_vector({2, 1, 0}), threes - one_two_three);
		TEST_EQUAL("{1, 2, 3} - {0, 0, 0}", one_two_three, one_two_three - zeroes);

		// - (negation)
		TEST_EQUAL("-{3, 3, 3}", test_vector({-3, -3, -3}), -threes);
		TEST_EQUAL("-{1, 2, 3}", test_vector({-1, -2, -3}), -one_two_three);

		// * (scalar multiply)
		TEST_EQUAL("{3, 3, 3}*5", test_vector({15, 15, 15}), threes * 5);
		TEST_EQUAL("{1, 2, 3}*5", test_vector({5, 10, 15}), one_two_three * 5);
		
		// * (dot product)
		TEST_EQUAL("zeroes dot zeroes", 0, zeroes * zeroes);
		TEST_EQUAL("threes dot threes", 9+9+9, threes * threes);
		TEST_EQUAL("one_two_three dot one_two_three", 1+4+9, one_two_three * one_two_three);
		TEST_EQUAL("threes dot one_two_three", 3+6+9, threes * one_two_three);

		// / (scalar divide)
		TEST_EQUAL("{3, 3, 3}/3", test_vector({1, 1, 1}), threes / 3);
		TEST_EQUAL("{100, 20, 40}/10", test_vector({10, 2, 4}), test_vector({100, 20, 40}) / 10);
		
	      });

  r.criterion("vector miscellaneous member functions",
	      1,
	      [&]() {
		// almost_equal
		TEST_TRUE("{3, 3, 3} almost equal {3, 3, 3}", threes.almost_equal(threes));
		TEST_FALSE("{3, 3, 3} almost equal {1, 2, 3}", threes.almost_equal(one_two_three));
		gfx::vector<double, 3> ones({1.0, 1.0, 1.0}), almost_ones({1.0, 1.0, 1.00001});
		TEST_TRUE("{1.0, 1.0, 1.0} almost equal {1.0, 1.0, 1.00001}", ones.almost_equal(almost_ones));

		// dimension
		TEST_EQUAL(".dimension() 2", 2, gfx::vector2<int>().dimension());
		TEST_EQUAL(".dimension() 3", 3, gfx::vector3<int>().dimension());
		TEST_EQUAL(".dimension() 4", 4, gfx::vector4<int>().dimension());

		// magnitude_squared
		TEST_EQUAL("magnitude_squared {0, 0, 0}", 0, zeroes.magnitude_squared());
		TEST_EQUAL("magnitude_squared {3, 3, 3}", 27, threes.magnitude_squared());
		TEST_EQUAL("magnitude_squared {1, 2, 3}", 1+4+9, one_two_three.magnitude_squared());

		// magnitude
		TEST_TRUE("magnitude {0, 0, 0}", gfx::almost_equal<double>(gfx::vector<double, 3>({0, 0, 0}).magnitude(),
									   0.0));
		TEST_TRUE("magnitude {1, 2, 3}", gfx::almost_equal<double>(gfx::vector<double, 3>({1, 2, 3}).magnitude(),
									   std::sqrt(1+4+9)));

		// normalize
		{
		  gfx::vector<double, 3> v({1.0, 2.0, 3.0});
		  double d = std::sqrt(1.0 + 4.0 + 9.0);
		  gfx::vector<double, 3> normalized({1.0 / d, 2.0 / d, 3.0 / d});
		  TEST_TRUE("normalize {1, 2, 3}", v.normalize().almost_equal(normalized));
		}

		// fill
		{
		  test_vector copy(one_two_three);
		  copy.fill(-1);
		  TEST_EQUAL("fill {1, 2, 3} with -1", test_vector({-1, -1, -1}), copy);
		}
	      });

  r.criterion("vector cross product",
	      1,
	      [&]() {
		// example from https://mathinsight.org/cross_product_examples
		TEST_EQUAL("{3, -3, 1} cross {4, 9, 2}",
			   test_vector({-15, -2, 39}),
			   test_vector({3, -3, 1}).cross(test_vector({4, 9, 2})));
		
		// example from http://tutorial.math.lamar.edu/Classes/CalcII/CrossProduct.aspx
		TEST_EQUAL("{0, 1, 1} cross {1, -1, 3}",
			   test_vector({4, 1, -1}),
			   test_vector({0, 1, 1}).cross(test_vector({1, -1, 3})));

		// example from https://www.mathsisfun.com/algebra/vectors-cross-product.html
		TEST_EQUAL("{2, 3, 4} cross {5, 6, 7}",
			   test_vector({-3, 6, -3}),
			   test_vector({2, 3, 4}).cross(test_vector({5, 6, 7})));
		
		// example from https://math.oregonstate.edu/home/programs/undergrad/CalculusQuestStudyGuides/vcalc/crossprod/crossprod.html
		TEST_EQUAL("{3, -2, -2} cross {-1, 0, 5}",
			   test_vector({-10, -13, -2}),
			   test_vector({3, -2, -2}).cross(test_vector({-1, 0, 5})));
	      });

  r.criterion("vector subvector, shrink, and grow",
	      1,
	      [&]() {
		int_vector<6> big({2, 3, 4, 5, 6, 7});
		int_vector<1> one({1});

		// subvector
		TEST_EQUAL("subvector length 1", int_vector<1>({2}), big.subvector<1>(0));
		TEST_EQUAL("subvector length 1", int_vector<1>({3}), big.subvector<1>(1));
		TEST_EQUAL("subvector length 1", int_vector<1>({4}), big.subvector<1>(2));
		TEST_EQUAL("subvector length 1", int_vector<1>({5}), big.subvector<1>(3));
		TEST_EQUAL("subvector length 1", int_vector<1>({6}), big.subvector<1>(4));
		TEST_EQUAL("subvector length 1", int_vector<1>({7}), big.subvector<1>(5));
		TEST_EQUAL("subvector length 2", int_vector<2>({2, 3}), big.subvector<2>(0));
		TEST_EQUAL("subvector length 2", int_vector<2>({3, 4}), big.subvector<2>(1));
		TEST_EQUAL("subvector length 2", int_vector<2>({4, 5}), big.subvector<2>(2));
		TEST_EQUAL("subvector length 2", int_vector<2>({5, 6}), big.subvector<2>(3));
		TEST_EQUAL("subvector length 2", int_vector<2>({6, 7}), big.subvector<2>(4));
		TEST_EQUAL("subvector length 3", int_vector<3>({2, 3, 4}), big.subvector<3>(0));
		TEST_EQUAL("subvector length 3", int_vector<3>({3, 4, 5}), big.subvector<3>(1));
		TEST_EQUAL("subvector length 3", int_vector<3>({4, 5, 6}), big.subvector<3>(2));
		TEST_EQUAL("subvector length 3", int_vector<3>({5, 6, 7}), big.subvector<3>(3));
		TEST_EQUAL("subvector length 4", int_vector<4>({2, 3, 4, 5}), big.subvector<4>(0));
		TEST_EQUAL("subvector length 4", int_vector<4>({3, 4, 5, 6}), big.subvector<4>(1));
		TEST_EQUAL("subvector length 4", int_vector<4>({4, 5, 6, 7}), big.subvector<4>(2));
		TEST_EQUAL("subvector length 5", int_vector<5>({2, 3, 4, 5, 6}), big.subvector<5>(0));
		TEST_EQUAL("subvector length 5", int_vector<5>({3, 4, 5, 6, 7}), big.subvector<5>(1));
		TEST_EQUAL("subvector length 6", int_vector<6>({2, 3, 4, 5, 6, 7}), big.subvector<6>(0));

		// shrink
		TEST_EQUAL("shrink to length 5", int_vector<5>({2, 3, 4, 5, 6}), big.shrink<5>());
		TEST_EQUAL("shrink to length 4", int_vector<4>({2, 3, 4, 5}), big.shrink<4>());
		TEST_EQUAL("shrink to length 3", int_vector<3>({2, 3, 4}), big.shrink<3>());
		TEST_EQUAL("shrink to length 2", int_vector<2>({2, 3}), big.shrink<2>());
		TEST_EQUAL("shrink to length 1", int_vector<1>({2}), big.shrink<1>());

		// grow
		TEST_EQUAL("grow to length 5", int_vector<5>({1, 2, 2, 2, 2}), one.grow<5>(2));
		TEST_EQUAL("grow to length 4", int_vector<4>({1, 2, 2, 2}), one.grow<4>(2));
		TEST_EQUAL("grow to length 3", int_vector<3>({1, 2, 2}), one.grow<3>(2));
		TEST_EQUAL("grow to length 2", int_vector<2>({1, 2}), one.grow<2>(2));
	      });

  const int_matrix_3x3 zero_matrix,
    ones_matrix(1),
    identity_matrix({1, 0, 0,
	             0, 1, 0,
	             0, 0, 1}),
    count_matrix({1, 2, 3,
	          4, 5, 6,
	          7, 8, 9}),
    copied_matrix(count_matrix);
  
  r.criterion("matrix constructor",
	      1,
	      [&]() {
		TEST_EQUAL("default constructor",
			   int_matrix_3x3({0, 0, 0,
				           0, 0, 0,
				           0, 0, 0}),
			   zero_matrix);

		TEST_EQUAL("fill constructor",
			   int_matrix_3x3({1, 1, 1,
				           1, 1, 1,
				           1, 1, 1}),
			   ones_matrix);

		TEST_EQUAL("initializer list constructor",
			   int_matrix_3x3({1, 0, 0,
				           0, 1, 0,
				           0, 0, 1}),
			   identity_matrix);

		TEST_EQUAL("copy constructor",
			   count_matrix,
			   copied_matrix);
	      });

  r.criterion("matrix operators except multiplication",
	      2,
	      [&]() {
		// =
		{
		  auto assigned = ones_matrix;
		  TEST_EQUAL("=", ones_matrix, assigned);
		  assigned = identity_matrix;
		  TEST_EQUAL("=", identity_matrix, assigned);
		}

		// ==
		TEST_TRUE("== true", zero_matrix == zero_matrix);
		TEST_TRUE("== true", copied_matrix == count_matrix);
		TEST_FALSE("== false", zero_matrix == ones_matrix);

		// !=
		TEST_TRUE("!= true", zero_matrix != ones_matrix);
		TEST_FALSE("!= false", copied_matrix != count_matrix);

		// <<
		std::stringstream ss;
		ss << count_matrix;
		TEST_EQUAL("<<",
			   "|1 2 3|\n|4 5 6|\n|7 8 9|\n",
			   ss.str());
		
		// const []
		TEST_EQUAL("[0][0]", 1, count_matrix[0][0]);
		TEST_EQUAL("[0][1]", 2, count_matrix[0][1]);
		TEST_EQUAL("[0][2]", 3, count_matrix[0][2]);
		TEST_EQUAL("[1][0]", 4, count_matrix[1][0]);
		TEST_EQUAL("[1][1]", 5, count_matrix[1][1]);
		TEST_EQUAL("[1][2]", 6, count_matrix[1][2]);
		TEST_EQUAL("[2][0]", 7, count_matrix[2][0]);
		TEST_EQUAL("[2][1]", 8, count_matrix[2][1]);
		TEST_EQUAL("[2][2]", 9, count_matrix[2][2]);

		// non-const []
		{
		  auto changed = count_matrix;
		  changed[0][0] = 0;
		  changed[2][2] = 0;
		  TEST_EQUAL("non-const []",
			     int_matrix_3x3({0, 2, 3,
				             4, 5, 6,
				             7, 8, 0}),
			     changed);
		}

		// +
		TEST_EQUAL("+",
			   zero_matrix,
			   zero_matrix + zero_matrix);
		TEST_EQUAL("+",
			   count_matrix,
			   count_matrix + zero_matrix);
		TEST_EQUAL("+",
			   int_matrix_3x3({2, 2, 3,
				           4, 6, 6,
				           7, 8, 10}),
			   count_matrix + identity_matrix);

		// - (binary operator)
		TEST_EQUAL("-",
			   zero_matrix,
			   zero_matrix - zero_matrix);
		TEST_EQUAL("-",
			   zero_matrix,
			   identity_matrix - identity_matrix);
		TEST_EQUAL("-",
			   int_matrix_3x3({0, 2, 3,
				           4, 4, 6,
				           7, 8, 8}),
			   count_matrix - identity_matrix);

		// - (negation)
		TEST_EQUAL("-",
			   zero_matrix,
			   -zero_matrix);
		TEST_EQUAL("-",
			   int_matrix_3x3({-1,  0,  0,
				            0, -1,  0,
				            0,  0, -1}),
			   -identity_matrix);
		TEST_EQUAL("-",
			   int_matrix_3x3({-1, -2, -3,
				           -4, -5, -6,
				           -7, -8, -9}),
			   -count_matrix);

		// / (scalar division)
		TEST_EQUAL("/",
			   count_matrix,
			   int_matrix_3x3({10, 20, 30,
				           40, 50, 60,
				           70, 80, 90}) / 10);
	      });

  r.criterion("matrix multiplication",
	      2,
	      [&]() {
		// matrix-scalar multiplication
		
		TEST_EQUAL("scalar multiply",
			   int_matrix_3x3({10, 20, 30,
				           40, 50, 60,
				 70, 80, 90}),
			   count_matrix * 10);		

		// matrix-matrix multiplication
		
		TEST_EQUAL("* identity",
			   ones_matrix,
			   ones_matrix * identity_matrix);
		TEST_EQUAL("* identity",
			   count_matrix,
			   count_matrix * identity_matrix);
		
		{
		  int_matrix<3, 2> lhs({0, 1,
			                2, 3,
			                4, 5});
		  int_matrix <2, 4> rhs({6, 7, 8, 9,
			                 0, 1, 2, 3});
		  int_matrix<3, 4> product({ 0,  1,  2,  3,
				            12, 17, 22, 27,
		                            24, 33, 42, 51});
		TEST_EQUAL("example on page 92 of the textbook",
			   product,
			   lhs * rhs);
		}

		{
		  // https://www.mathsisfun.com/algebra/matrix-multiplying.html
		  int_matrix<2, 3> lhs({1, 2, 3,
			                4, 5, 6});
		  int_matrix<3, 2> rhs({ 7,  8,
			                 9, 10,
			                11, 12});
		  int_matrix<2, 2> product({58, 64,
			                    139, 154});
		  TEST_EQUAL("example from mathisfun.com",
			     product,
			     lhs * rhs);
		}
	      });

  r.criterion("matrix submatrix, shrink, grow",
	      1,
	      [&]() {
		// submatrix
		auto whole_thing = count_matrix.submatrix<3, 3>(0, 0);
		TEST_EQUAL("whole thing", count_matrix, whole_thing);

		auto left_side_expected = int_matrix<3, 2>({1, 2,
		      4, 5,
		      7, 8}),
		  left_side_got = count_matrix.submatrix<3, 2>(0, 0);
		TEST_EQUAL("left side", left_side_expected, left_side_got);

		auto bottom_half_expected = int_matrix<2, 3>({4, 5, 6,
		      7, 8, 9}),
		  bottom_half_got = count_matrix.submatrix<2, 3>(1, 0);
		TEST_EQUAL("bottom half", bottom_half_expected, bottom_half_got);

		auto top_left_expected = int_matrix_2x2({1, 2, 4, 5}),
		  top_left_got = count_matrix.submatrix<2, 2>(0, 0),
		  top_right_expected = int_matrix_2x2({2, 3, 5, 6}),
		  top_right_got = count_matrix.submatrix<2, 2>(0, 1),
		  bottom_left_expected = int_matrix_2x2({4, 5, 7, 8}),
		  bottom_left_got = count_matrix.submatrix<2, 2>(1, 0),
		  bottom_right_expected = int_matrix_2x2({5, 6, 8, 9}),
		  bottom_right_got = count_matrix.submatrix<2, 2>(1, 1);
		TEST_EQUAL("top left", top_left_expected, top_left_got);
		TEST_EQUAL("top right", top_right_expected, top_right_got);
		TEST_EQUAL("bottom left", bottom_left_expected, bottom_left_got);
		TEST_EQUAL("bottom right", bottom_right_expected, bottom_right_got);

		// shrink
		auto one_by_one_expected = int_matrix<1, 1>({1}),
		  one_by_one_got = count_matrix.shrink<1, 1>();
		auto two_by_two_expected = int_matrix<2, 2>({1, 2, 4, 5}),
		  two_by_two_got = count_matrix.shrink<2, 2>();
		TEST_EQUAL("shrink to 1x1", one_by_one_expected, one_by_one_got);
		TEST_EQUAL("shrink to 2x2", two_by_two_expected, two_by_two_got);

		// grow
		auto grow_default_expected = int_matrix<4, 4>({1, 2, 3, 0,
		      4, 5, 6, 0,
		      7, 8, 9, 0,
		      0, 0, 0, 0}),
		  grow_default_got = count_matrix.grow<4, 4>();
		TEST_EQUAL("grow to 4x4 with default fill",
			   grow_default_expected,
			   grow_default_got);
		auto grow_fill_expected = int_matrix<4, 3>({1, 2, 3,
		      4, 5, 6,
		      7, 8, 9,
		      11, 11, 11}),
		  grow_fill_got = count_matrix.grow<4, 3>(11);
		TEST_EQUAL("grow to 4x3 with fill 11",
			   grow_fill_expected,
			   grow_fill_got);
	      });

  r.criterion("matrix miscellaneous member functions",
	      2,
	      [&]() {
		// almost_equal
		{
		  gfx::matrix<double, 2, 3> whole({1, 2, 3,
			                           4, 5, 6}),
		    perturbed({1, 2, 3,
			       4, 5, 6.0001});
		  TEST_TRUE("almost equal",
			    whole.almost_equal(perturbed));
		}

		// column_matrix
		auto left_column_matrix = int_matrix<3, 1>({1, 4, 7});
		TEST_EQUAL("column_matrix",
			   left_column_matrix,
			   count_matrix.column_matrix(0));

		// column vector
		TEST_EQUAL("column_vector",
			   test_vector({1, 4, 7}),
			   count_matrix.column_vector(0));

		// height
		TEST_EQUAL("height", 3, identity_matrix.height());


		// identity
		auto id2_expected = int_matrix<2, 2>({1, 0, 0, 1}),
		  id2_got = int_matrix<2, 2>::identity();
		auto id3_got = int_matrix<3, 3>::identity();
		auto id4_expected = int_matrix<4, 4>({1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
		      0, 0, 0, 1}),
		  id4_got = int_matrix<4, 4>::identity();
		TEST_EQUAL("2x2 identity", id2_expected, id2_got);
		TEST_EQUAL("3x3 identity", identity_matrix, id3_got);
		TEST_EQUAL("4x4 identity", id4_expected, id4_got);

		// row_matrix
		auto bottom_row_matrix = int_matrix<1, 3>({7, 8, 9});
		TEST_EQUAL("row_matrix",
			   bottom_row_matrix,
			   count_matrix.row_matrix(2));

		// row_vector
		auto bottom_row_vector = int_vector<3>({7, 8, 9});
		TEST_EQUAL("row_vector",
			   bottom_row_vector,
			   count_matrix.row_vector(2));

		// transpose
		auto count_transposed = int_matrix<3, 3>({1, 4, 7,
		      2, 5, 8,
		      3, 6, 9});
		TEST_EQUAL("transpose",
			   count_transposed,
			   count_matrix.transpose());

		// width
		TEST_EQUAL("width", 3, identity_matrix.width());
	      });

  r.criterion("matrix determinant",
	      2,
	      [&]() {
		// 2x2 example from https://www.mathsisfun.com/algebra/matrix-determinant.html
		{
		  int_matrix_2x2 m({4, 6, 3, 8});
		  TEST_EQUAL("2x2 mathisfun example", 14, m.determinant());
		}

		// 2x2 examples from https://www.chilimath.com/lessons/advanced-algebra/determinant-2x2-matrix/
		{
		  int_matrix_2x2 m1({1, 2, 3, 4});
		  TEST_EQUAL("2x2 chilimath example 1", -2, m1.determinant());

		  int_matrix_2x2 m2({-5, -4, -2, -3});
		  TEST_EQUAL("2x2 chilimath example 2", 7, m2.determinant());
		  
		  int_matrix_2x2 m3({-1, -2, 6, 3});
		  TEST_EQUAL("2x2 chilimath example 3", 9, m3.determinant());
		  
		  int_matrix_2x2 m4({-4, 2, -8, 7});
		  TEST_EQUAL("2x2 chilimath example 4", -12, m4.determinant());
		}
		
		// 3x3 example on page 98 of textbook
		{
		  int_matrix_3x3 m({0, 1, 2, 3, 4, 5, 6, 7, 8});
		  TEST_EQUAL("p.98 3x3 example", 0, m.determinant());
		}
		// 3x3 example from https://www.mathsisfun.com/algebra/matrix-determinant.html
		{
		  int_matrix_3x3 m({6, 1, 1, 4, -2, 5, 2, 8, 7});
		  TEST_EQUAL("3x3 mathisfun example", -306, m.determinant());
		}
		// 3x3 examples from https://www.chilimath.com/lessons/advanced-algebra/determinant-3x3-matrix/
		{
		  int_matrix_3x3 m1({2, -3, 1, 2, 0, -1, 1, 4, 5});
		  TEST_EQUAL("3x3 chilimath example 1", 49, m1.determinant());

		  int_matrix_3x3 m2({1, 3, 2, -3, -1, -3, 2, 3, 1});
		  TEST_EQUAL("3x3 chilimath example 2", -15, m2.determinant());

		  int_matrix_3x3 m3({-5, 0, -1, 1, 2, -1, -3, 4, 1});
		  TEST_EQUAL("3x3 chilimath example 3", -40, m3.determinant());
		}
	      });

  r.criterion("matrix solve",
	      2,
	      [&]() {
		// 2x2 examples from https://www.chilimath.com/lessons/advanced-algebra/cramers-rule-with-two-variables/
		// example 1
		{
		  int_matrix_2x2 m({4, -3, 6, 5});
		  gfx::vector<int, 2> b({11, 7}),
		    expected({2, -1}),
		    got = m.solve(b);
		  TEST_EQUAL("2x2 example 1", expected, got);
		}
		// example 2
		{
		  int_matrix_2x2 m({3, 5, 1, 4});
		  gfx::vector<int, 2> b({-7, -14}),
		    expected({6, -5}),
		    got = m.solve(b);
		  TEST_EQUAL("2x2 example 2", expected, got);
		}
		// example 3
		{
		  int_matrix_2x2 m({1, -4, -1, 5});
		  gfx::vector<int, 2> b({-9, 11}),
		    expected({-1, 2}),
		    got = m.solve(b);
		  TEST_EQUAL("2x2 example 3", expected, got);
		}
		// example 4
		{
		  int_matrix_2x2 m({-2, 3, 3, -4});
		  gfx::vector<int, 2> b({-3, 5}),
		    expected({3, 1}),
		    got = m.solve(b);
		  TEST_EQUAL("2x2 example 4", expected, got);
		}
		// example 5
		{
		  int_matrix_2x2 m({5, 1, 3, -2});
		  gfx::vector<int, 2> b({-13, 0}),
		    expected({-2, -3}),
		    got = m.solve(b);
		  TEST_EQUAL("2x2 example 5", expected, got);
		}
		
		// 3x3 examples from https://www.chilimath.com/lessons/advanced-algebra/cramers-rule-with-three-variables/
		// example 1
		{
		  int_matrix_3x3 m({1, 2, 3, 3, 1, -3, -3, 4, 7});
		  test_vector b({-5, 4, -7}),
		    expected({-1, 1, -2}),
		    got = m.solve(b);
		  TEST_EQUAL("3x3 example 1", expected, got);
		}
		// example 2
		{
		  int_matrix_3x3 m({-2, -1, -3, 2, -3, 1, 2, 0, -3});
		  test_vector b({3, -13, -11}),
		    expected({-4, 2, 1}),
		    got = m.solve(b);
		  TEST_EQUAL("3x3 example 2", expected, got);
		}
		// example 3
		{
		  int_matrix_3x3 m({0, -1, -2, 1, 0, 3, 7, 1, 1});
		  test_vector b({-8, 2, 0}),
		    expected({-1, 6, 1}),
		    got = m.solve(b);
		  TEST_EQUAL("3x3 example 3", expected, got);
		}
		// example 4
		{
		  int_matrix_3x3 m({-2, 1, 1, -4, 2, -1, -6, -3, 1});
		  test_vector b({4, 8, 0}),
		    expected({-1, 2, 0}),
		    got = m.solve(b);
		  TEST_EQUAL("3x3 example 4", expected, got);
		}
		// example 5
		{
		  gfx::matrix<double, 3, 3> m({1, -8, 1, -1, 2, 1, 1, -1, 2});
		  gfx::vector<double, 3> b({4, 2, -1}),
		    expected({-3, -4.0/5.0, 3.0/5.0}),
		    got = m.solve(b);
		  TEST_TRUE("3x3 example 4", expected.almost_equal(got));
		}
		
	      });
  
  return r.run();
}


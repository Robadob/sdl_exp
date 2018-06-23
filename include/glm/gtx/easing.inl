///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Mathematics (glm.g-truc.net)
///
/// Copyright (c) 2005 - 2015 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// Restrictions:
///		By making use of the Software for military purposes, you choose to make
///		a Bunny unhappy.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref gtx_easing
/// @file glm/gtx/easing.inl
/// @date 2018-05-05
/// @author Robert Chisholm
/// @note Based on the AHEasing project of Warren Moore (https://github.com/warrenm/AHEasing)
///////////////////////////////////////////////////////////////////////////////////

#include <cmath>

namespace glm{

	template <typename genType>
	GLM_FUNC_QUALIFIER genType linearInterpolation(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return a;
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quadraticEaseIn(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return a * a;
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quadraticEaseOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return -(a * (a - 2));
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quadraticEaseInOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if(a < 0.5)
		{
			return 2 * a * a;
		}
		else
		{
			return (-2 * a * a) + (4 * a) - one<genType>();
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType cubicEaseIn(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return a * a * a;
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType cubicEaseOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		genType const f = a - one<genType>();
		return f * f * f + one<genType>();
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType cubicEaseInOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if (a < 0.5)
		{
			return 4 * a * a * a;
		}
		else
		{
			genType const f = ((2 * a) - 2);
			return 0.5f * f * f * f + one<genType>();
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quarticEaseIn(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return a * a * a * a;
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quarticEaseOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		genType const f = (a - one<genType>());
		return f * f * f * (one<genType>() - a) + one<genType>();
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quarticEaseInOut(genType const & a) 
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if (a < 0.5)
		{
			return 8 * a * a * a * a;
		}
		else
		{
			genType const f = (a - one<genType>());
			return -8 * f * f * f * f + one<genType>();
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quinticEaseIn(genType const & a) 
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return a * a * a * a * a;
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quinticEaseOut(genType const & a) 
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		genType const f = (a - one<genType>());
		return f * f * f * f * f + one<genType>();
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType quinticEaseInOut(genType const & a) 
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if (a < 0.5)
		{
			return 16 * a * a * a * a * a;
		}
		else
		{
			genType const f = ((2 * a) - 2);
			return 0.5f * f * f * f * f * f + one<genType>();
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType sineEaseIn(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return sin((a - one<genType>()) * half_pi<genType>()) + one<genType>();
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType sineEaseOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return sin(a * half_pi<genType>());
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType sineEaseInOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return 0.5f * (one<genType>() - cos(a * pi<genType>()));
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType circularEaseIn(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return one<genType>() - sqrt(one<genType>() - (a * a));
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType circularEaseOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return sqrt((2 - a) * a);
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType circularEaseInOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if(a < 0.5)
		{
			return 0.5f * (one<genType>() - sqrt(one<genType>() - 4 * (a * a)));
		}
		else
		{
			return 0.5f * (sqrt(-((2 * a) - 3) * ((2 * a) - one<genType>())) + one<genType>());
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType exponentialEaseIn(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return (a == zero<genType>()) ? a : pow<genType>(2, 10 * (a - one<genType>()));
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType exponentialEaseOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return (a == one<genType>()) ? a : one<genType>() - pow<genType>(2, -10 * a);
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType exponentialEaseInOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if(a == zero<genType>() || a == one<genType>()) return a;
		
		if(a < 0.5)
		{
			return 0.5f * pow<genType>(2, (20 * a) - 10);
		}
		else
		{
			return -0.5f * pow<genType>(2, (-20 * a) + 10) + one<genType>();
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType elasticEaseIn(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return sin(13 * half_pi<genType>() * a) * pow<genType>(2, 10 * (a - one<genType>()));
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType elasticEaseOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		return sin(-13 * half_pi<genType>() * (a + one<genType>())) * pow<genType>(2, -10 * a) + one<genType>();
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType elasticEaseInOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if(a < 0.5)
		{
			return 0.5f * sin(13 * half_pi<genType>() * (2 * a)) * pow<genType>(2, 10 * ((2 * a) - one<genType>()));
		}
		else
		{
			return 0.5f * (sin(-13 * half_pi<genType>() * ((2 * a - one<genType>()) + one<genType>())) * pow<genType>(2, -10 * (2 * a - one<genType>())) + 2);
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType backEaseIn(genType const & a, genType const & o)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());

		float z = ((o + one<genType>()) * a) - o;
		return (a*a*z);
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType backEaseOut(genType const & a, genType const & o)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());

		float n = a - one<genType>();
		float z = ((o + one<genType>()) * n) + o;
		return (n*n*z) + one<genType>();
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType backEaseInOut(genType const & a, genType const & o)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());

		float s = o * 1.525f;
		float x = 0.5f;
		float n = a / 0.5f;

		if (n < 1.0f){
			float z = ((s + 1.0f) * n) - s;
			float m = n*n*z;
			return x * m;
		}
		else 
		{
			n -= 2.0f;
			float z = ((s + 1.0f) * n) + s;
			float m = (n*n*z) + 2.0f;
			return x * m;
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType bounceEaseOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if(a < 4/11.0f)
		{
			return (121 * a * a)/16.0f;
		}
		else if(a < 8/11.0)
		{
			return (363/40.0f * a * a) - (99/10.0f * a) + 17/5.0f;
		}
		else if(a < 9/10.0)
		{
			return (4356/361.0f * a * a) - (35442/1805.0f * a) + 16061/1805.0f;
		}
		else
		{
			return (54/5.0f * a * a) - (513/25.0f * a) + 268/25.0f;
		}
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType bounceEaseIn(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());

		return one<genType>() - bounceEaseOut(one<genType>() - a);
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER genType bounceEaseInOut(genType const & a)
	{
		// Only defined in [0, 1]
		assert(a >= zero<genType>());
		assert(a <= one<genType>());
    
		if(a < 0.5)
		{
			return 0.5f * (one<genType>() - bounceEaseOut(a * 2));
		}
		else
		{
			return 0.5f * bounceEaseOut(a * 2 - one<genType>()) + 0.5f;
		}
	}
  
}//namespace glm
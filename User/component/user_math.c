/*
  自定义的数学运算。
*/

#include "user_math.h"

#include <string.h>

inline float InvSqrt(float x) {
#if 0
  /* Fast inverse square-root */
  /* See: http://en.wikipedia.org/wiki/Fast_inverse_square_root */
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	y = y * (1.5f - (halfx * y * y));
	return y;
#else
  return 1.0f / sqrtf(x);
#endif
}

inline float AbsClip(float in, float limit) {
  return (in < -limit) ? -limit : ((in > limit) ? limit : in);
}

inline float Sign(float in) { return (in > 0) ? 1.0f : 0.0f; }

inline void ResetMoveVector(MoveVector_t *mv) {
  memset(mv, 0, sizeof(MoveVector_t));
}

inline float CircleError(float sp, float fb, float range) {
  float error = sp - fb;
  if (range > 0.0f) {
    float half_range = range / 2.0f;
    if (error > half_range) {
			error -= range;
    }else if(error < -half_range)
			error += range;
  }
  return error;
}

inline void CircleAdd(float *origin, float delta, float range) {
  float out = *origin + delta;
  if (out > range)
    out -= range;
  else if (out < 0.0f)
    out += range;

  *origin = out;
}

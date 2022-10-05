#include "s21_decimal.h"

int s21_get_bit_decimal(s21_decimal num, int pos) {
  int mask = 1U << (pos % 32);
  return !!(num.bits[pos / 32] & mask);
}

int s21_get_bit_super_decimal(s21_super_decimal s_num, int pos) {
  int mask = 1U << (pos % 32);
  return !!(s_num.bits[pos / 32] & mask);
}

void s21_set_bit_decimal(s21_decimal *num, int pos) {
  int mask = 1U << (pos % 32);
  num->bits[pos / 32] |= mask;
}

void s21_set_bit_super_decimal(s21_super_decimal *s_num, int pos) {
  int mask = 1U << (pos % 32);
  s_num->bits[pos / 32] |= mask;
}

void s21_clear_bit_decimal(s21_decimal *num, int pos) {
  int mask = 1U << (pos % 32);
  num->bits[pos / 32] &= ~mask;
}

void s21_clear_bit_super_decimal(s21_super_decimal *s_num, int pos) {
  int mask = 1U << (pos % 32);
  s_num->bits[pos / 32] &= ~mask;
}

void s21_init_bit_decimal(s21_decimal *num, int pos, int bit) {
  if (bit) {
    s21_set_bit_decimal(num, pos);
  } else {
    s21_clear_bit_decimal(num, pos);
  }
}

void s21_init_bit_super_decimal(s21_super_decimal *s_num, int pos, int bit) {
  if (bit) {
    s21_set_bit_super_decimal(s_num, pos);
  } else {
    s21_clear_bit_super_decimal(s_num, pos);
  }
}

int s21_get_sign_decimal(s21_decimal num) {
  return (unsigned)num.bits[SCALE] >> 31;
}

int s21_get_sign_super_decimal(s21_super_decimal s_num) {
  return (unsigned)s_num.bits[S_SCALE] >> 31;
}

void s21_set_sign_decimal(s21_decimal *num, int sign) {
  num->bits[SCALE] &= ~(1U << 31);
  num->bits[SCALE] |= (unsigned)sign << 31;
}

void s21_set_sign_super_decimal(s21_super_decimal *s_num, int sign) {
  s_num->bits[S_SCALE] &= ~(1U << 31);
  s_num->bits[S_SCALE] |= (unsigned)sign << 31;
}

int s21_get_scale_decimal(s21_decimal num) {
  return (unsigned)(num.bits[SCALE] << 8) >> 24;
}

int s21_get_scale_super_decimal(s21_super_decimal s_num) {
  return (unsigned)(s_num.bits[S_SCALE] << 8) >> 24;
}

void s21_set_scale_decimal(s21_decimal *num, int decimalPow) {
  num->bits[SCALE] &= 1U << 31;
  num->bits[SCALE] |= decimalPow << 16;
}

void s21_set_scale_super_decimal(s21_super_decimal *s_num, int decimalPow) {
  s_num->bits[S_SCALE] &= 1U << 31;
  s_num->bits[S_SCALE] |= decimalPow << 16;
}

int s21_get_sign_from_float(int num) { return (unsigned)num >> 31; }

int s21_get_exp_from_float(int num) {
  return (unsigned)(num & 0x7f800000) >> 23;
}

int s21_get_mantiss_from_float(int num) { return (num & 0x007fffff); }

s21_decimal s21_set_zero_decimal() {
  s21_decimal res;
  res.bits[LOW] = 0;
  res.bits[MID] = 0;
  res.bits[HIGH] = 0;
  res.bits[SCALE] = 0;
  return res;
}

s21_super_decimal s21_set_zero_super_decimal() {
  s21_super_decimal s_res;
  s_res.bits[LOWS] = 0;
  s_res.bits[MIDS] = 0;
  s_res.bits[HIGHS] = 0;
  s_res.bits[S_LOWS] = 0;
  s_res.bits[S_MIDS] = 0;
  s_res.bits[S_HIGHS] = 0;
  s_res.bits[S_SCALE] = 0;
  return s_res;
}

s21_decimal s21_init_decimal() {
  s21_decimal res;
  res = s21_set_zero_decimal();
  res.value_type = s21_NORMAL_VALUE;
  return res;
}

int s21_check_decimal_to_zero(s21_decimal num) {
  int flag_zero = 1;
  if (!num.bits[HIGH] && !num.bits[MID] && !num.bits[LOW]) {
    flag_zero = 0;
  }
  return flag_zero;
}

int s21_check_super_decimal_to_zero(s21_super_decimal s_num) {
  int flag_zero = 1;
  if (!s_num.bits[S_HIGHS] && !s_num.bits[S_MIDS] && !s_num.bits[S_LOWS] &&
      !s_num.bits[HIGHS] && !s_num.bits[MIDS] && !s_num.bits[LOWS]) {
    flag_zero = 0;
  }
  return flag_zero;
}

// Возвращаемое значение - код ошибки:
// 0 - SUCCESS, 1 - CONVERTING ERROR
// Ошибка: |x| > 7.9e28, 0 < |x| < 1e-28, x == +-INF
int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  int flag_err = 0;
  int data = *((unsigned *)&src);
  int sign = s21_get_sign_from_float(data);
  int exp = s21_get_exp_from_float(data);
  int mantiss = s21_get_mantiss_from_float(data);
  if (exp == 0xFF) {  // Граничные условия: NAN, INF
    if (mantiss != 0) {
      *dst = s21_set_zero_decimal();
      dst->value_type = s21_NAN;
    } else {
      *dst = s21_set_infinity_decimal(sign);
    }
    flag_err = 1;
  } else if (data == 0) {
    *dst = s21_init_decimal();
  } else if ((unsigned)data == 0x80000000) {
    *dst = s21_init_decimal();
    s21_set_sign_decimal(dst, 1);
  } else {
    *dst = s21_init_decimal();
    s21_super_decimal buf = SUPER_DECIMAL_DEFAULT;
    buf.bits[LOWS] = mantiss + (1 << 23);
    exp = exp - 127 - 23;
    if (exp < 0) {
      int scale;
      for (; exp < 0 && s21_get_bit_super_decimal(buf, 0) == 0; exp++) {
        buf = s21_shift_to_right_super_decimal(buf);
      }
      s21_super_decimal five = s21_init_int_super_decimal(5);
      for (scale = 0; exp < 0 && scale < 28; exp++, scale++) {
        buf = s21_mul_with_check_signs(buf, five);
      }
      s21_set_scale_super_decimal(&buf, scale);
      for (; exp < 0; exp++) {
        buf = s21_shift_to_right_super_decimal(buf);
      }
    } else if (exp > 0) {
      buf = s21_shift_to_left_super_decimal(buf, exp);
      buf = s21_increase_super_decimal_to_max_scale(buf);
    }
    if (buf.value_type != s21_NORMAL_VALUE) {
      s21_set_infinity_super_decimal(sign);
      flag_err = 1;
    } else {
      s21_set_sign_super_decimal(&buf, sign);
    }
    *dst = s21_convert_superdecimal_to_decimal(buf);
    if (s21_check_decimal_to_zero(*dst) == 0) {
      flag_err = 1;
    }
    sign = s21_get_sign_decimal(*dst);
    s21_set_sign_decimal(dst, 0);
    s21_decimal tmp = *dst;
    s21_decimal zero = DECIMAL_DEFAULT;
    int i = 0;
    while (s21_is_not_equal(tmp, zero) == 0) {
      tmp = s21_div_by_10_decimal(tmp);
      i++;
    }
    int scale = s21_get_scale_decimal(*dst);
    int multiplex = 0;
    if (i > scale && i > 7) {
      multiplex = i - scale + 1;
      i -= 8;
      multiplex -= 8;
    } else if (i >= 8) {
      i -= 8;
    } else {
      i = 0;
    }
    if (i > 0) {
      for (; i > 0; i--) {
        *dst = s21_div_by_10_decimal(*dst);
      }
      s21_decimal mod = s21_mod_ten_decimal(*dst);
      int mod_int = 0;
      s21_from_decimal_to_int(mod, &mod_int);
      *dst = s21_div_by_10_decimal(*dst);
      if (mod_int >= 5) {
        s21_decimal one = s21_init_int_decimal(1);
        s21_set_scale_decimal(&one, s21_get_scale_decimal(*dst));
        *dst = s21_add(*dst, one);
      }
      for (; multiplex > 0; multiplex--) {
        *dst = s21_mul_by_10_decimal(*dst);
      }
    }
    s21_set_sign_decimal(dst, sign);
  }
  return flag_err;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int error = 1;
  if (src.value_type == s21_NORMAL_VALUE) {
    double temp = 0.0;
    for (int i = 0; i < 96; i++) {
      if (s21_get_bit_decimal(src, i)) temp += pow(2, i);
    }
    if (s21_get_scale_decimal(src) != 0) {
      for (int i = s21_get_scale_decimal(src); i > 0; i--) temp /= 10.0;
    }
    *dst = (float)temp;
    if (s21_get_sign_decimal(src) == 1) *dst *= -1;
    error = 0;
  }
  return error;
}

s21_decimal s21_set_infinity_decimal(int sign) {
  s21_decimal res = s21_set_zero_decimal();
  if (sign == 1) {
    res.value_type = s21_NEGATIVE_INFINITY;
  } else {
    res.value_type = s21_INFINITY;
  }
  return res;
}

s21_super_decimal s21_set_infinity_super_decimal(int sign) {
  s21_super_decimal s_res = s21_set_zero_super_decimal();
  if (sign == 1) {
    s_res.value_type = s21_NEGATIVE_INFINITY;
  } else {
    s_res.value_type = s21_INFINITY;
  }
  return s_res;
}

s21_decimal s21_shift_to_left_decimal(s21_decimal num, int shift) {
  s21_decimal res = DECIMAL_DEFAULT;
  res.bits[SCALE] = num.bits[SCALE];
  int flag_inf = 0;
  for (int pos = 95; flag_inf == 0 && pos >= 0; pos--) {
    if ((pos > 95 - shift) && s21_get_bit_decimal(num, pos)) {
      // если какой-то из shift самых левых битов равен 1,
      // то происходит переполнение
      flag_inf = 1;
      res = s21_set_infinity_decimal(s21_get_sign_decimal(num));
    } else if (pos >= shift) {
      // узнаем бит, смещенный на shift битов вправо
      int bit = s21_get_bit_decimal(num, pos - shift);
      // устанавливаем этот бит в текущее положение
      s21_init_bit_decimal(&res, pos, bit);
    }
    // оставшиеся правые биты уже занулены
  }
  return res;
}

s21_super_decimal s21_shift_to_left_super_decimal(s21_super_decimal s_num,
                                                  int shift) {
  s21_super_decimal s_res = SUPER_DECIMAL_DEFAULT;
  if (s_num.value_type == s21_NORMAL_VALUE) {
    s_res.bits[S_SCALE] = s_num.bits[S_SCALE];
    int flag_inf = 0;
    for (int pos = 191; !flag_inf && pos >= 0; pos--) {
      if ((pos > 191 - shift) && s21_get_bit_super_decimal(s_num, pos)) {
        // если какой-то из shift самых левых битов равен 1,
        // то происходит переполнение
        flag_inf = 1;
        s_res =
            s21_set_infinity_super_decimal(s21_get_sign_super_decimal(s_num));
      } else if (pos >= shift) {
        // узнаем бит, смещенный на shift битов вправо
        int bit = s21_get_bit_super_decimal(s_num, pos - shift);
        // устанавливаем этот бит в текущее положение
        s21_init_bit_super_decimal(&s_res, pos, bit);
      }
      // оставшиеся правые биты уже занулены
    }
  } else {
    s_res.value_type = s_num.value_type;
  }
  return s_res;
}

s21_super_decimal s21_init_max_value_div_10_super_decimal() {
  s21_super_decimal s_max_value_div_10;
  s_max_value_div_10.value_type = s21_NORMAL_VALUE;
  s_max_value_div_10.bits[LOWS] = 2576980377;
  s_max_value_div_10.bits[MIDS] = 2576980377;
  s_max_value_div_10.bits[HIGHS] = 2576980377;
  s_max_value_div_10.bits[S_LOWS] = 2576980377;
  s_max_value_div_10.bits[S_MIDS] = 2576980377;
  s_max_value_div_10.bits[S_HIGHS] = 429496729;
  s_max_value_div_10.bits[S_SCALE] = 0;
  return s_max_value_div_10;
}

s21_super_decimal s21_copy_superdecimal_with_S_SCALE_0(
    s21_super_decimal s_num) {
  s21_super_decimal s_res = s_num;
  s_res.bits[S_SCALE] = 0;
  return s_res;
}

s21_super_decimal s21_increase_super_decimal_to_max_scale(
    s21_super_decimal s_num) {
  s21_super_decimal s_res = s_num;
  int scale = s21_get_scale_super_decimal(s_res);
  s21_super_decimal compare = s21_init_max_value_div_10_super_decimal();
  s21_super_decimal tmp = s21_copy_superdecimal_with_S_SCALE_0(s_res);
  for (; scale < 56 && s21_is_greater_or_equal_super_decimal(tmp, compare) == 1;
       scale++) {
    s_res = s21_mul_by_10_super_decimal(s_res);
    tmp = s21_copy_superdecimal_with_S_SCALE_0(s_res);
  }
  s21_set_scale_super_decimal(&s_res, scale);
  return s_res;
}

s21_super_decimal s21_shift_to_right_super_decimal(s21_super_decimal s_num) {
  s21_super_decimal s_res = s_num;
  if (s_res.value_type == s21_NORMAL_VALUE) {
    for (int i = 0; i < 5; i++) {
      s_res.bits[i] = (unsigned)s_res.bits[i] >> 1;
      s_res.bits[i] |= ((s_res.bits[i + 1] & 1U) << 31);
    }
    s_res.bits[5] = (unsigned)s_res.bits[5] >> 1;
  }
  return s_res;
}

s21_decimal s21_add(s21_decimal num1, s21_decimal num2) {
  s21_decimal sum = DECIMAL_DEFAULT;
  if (s21_check_add(num1, num2, &sum) == 0) {
    s21_super_decimal s_num1 = s21_convert_decimal_to_superdecimal(num1);
    s21_super_decimal s_num2 = s21_convert_decimal_to_superdecimal(num2);
    s21_normalisation_exponents(&s_num1, &s_num2);
    s21_super_decimal s_sum = s21_add_with_check_signs(s_num1, s_num2);
    sum = s21_convert_superdecimal_to_decimal(s_sum);
    if (s21_check_decimal_to_zero(num1) == 0 &&
        s21_check_decimal_to_zero(num2) == 0) {
      int scale = s21_get_scale_decimal(num1) >= s21_get_scale_decimal(num2)
                      ? s21_get_scale_decimal(num1)
                      : s21_get_scale_decimal(num2);
      s21_set_scale_decimal(&sum, scale);
    }
  }
  return sum;
}

int s21_check_add(s21_decimal num1, s21_decimal num2, s21_decimal *sum) {
  int error = 0;
  if (num1.value_type == s21_NAN || num2.value_type == s21_NAN ||
      (num1.value_type == s21_INFINITY &&
       num2.value_type == s21_NEGATIVE_INFINITY) ||
      (num1.value_type == s21_NEGATIVE_INFINITY &&
       num2.value_type == s21_INFINITY)) {
    sum->value_type = s21_NAN;
    error = 1;
  } else if (num1.value_type == s21_INFINITY ||
             num2.value_type == s21_INFINITY) {
    *sum = s21_set_infinity_decimal(0);
    error = 1;
  } else if (num1.value_type == s21_NEGATIVE_INFINITY ||
             num2.value_type == s21_NEGATIVE_INFINITY) {
    *sum = s21_set_infinity_decimal(1);
    error = 1;
  }
  return error;
}

s21_super_decimal s21_add_with_check_signs(s21_super_decimal s_num1,
                                           s21_super_decimal s_num2) {
  s21_super_decimal s_sum;
  int sign = 0, scale = s21_get_scale_super_decimal(s_num1);
  s21_set_scale_super_decimal(&s_num1, 0);
  s21_set_scale_super_decimal(&s_num2, 0);
  if (s21_get_sign_super_decimal(s_num1) == 0 &&
      s21_get_sign_super_decimal(s_num2) == 0) {
    s_sum = s21_add_without_verify(s_num1, s_num2, 0);
  } else if (s21_get_sign_super_decimal(s_num1) == 1 &&
             s21_get_sign_super_decimal(s_num2) == 1) {
    s21_set_sign_super_decimal(&s_num1, 0);
    s21_set_sign_super_decimal(&s_num2, 0);
    s_sum = s21_add_without_verify(s_num1, s_num2, 0);
    sign = 1;
    s21_set_sign_super_decimal(&s_sum, sign);
  } else if (s21_get_sign_super_decimal(s_num1) == 0 &&
             s21_get_sign_super_decimal(s_num2) == 1) {
    s21_set_sign_super_decimal(&s_num2, 0);
    s_sum = s21_sub_without_verify(s_num1, s_num2);
  } else {
    s21_set_sign_super_decimal(&s_num1, 0);
    s_sum = s21_sub_without_verify(s_num2, s_num1);
  }
  if (s_sum.value_type != s21_NORMAL_VALUE) {
    s_sum = s21_set_infinity_super_decimal(sign);
  } else {
    s21_set_scale_super_decimal(&s_sum, scale);
  }
  return s_sum;
}

// dop_code: 0 - без доп.кода, 1 - сложение в доп.коде
// складывать можно только 2 положительных числа
s21_super_decimal s21_add_without_verify(s21_super_decimal s_num1,
                                         s21_super_decimal s_num2,
                                         int dop_code) {
  int carry = 0;  // число, которое переходит в следующий разряд
  s21_super_decimal s_sum = SUPER_DECIMAL_DEFAULT;
  for (int i = 0; i < 192; i++) {
    int tmp = s21_get_bit_super_decimal(s_num1, i) +
              s21_get_bit_super_decimal(s_num2, i) + carry;
    s21_init_bit_super_decimal(&s_sum, i, (tmp & 1));
    carry = tmp >> 1;
  }
  if (carry && dop_code == 0) {
    s_sum.value_type = s21_INFINITY;
  }
  return s_sum;
}

s21_decimal s21_sub(s21_decimal num1, s21_decimal num2) {
  s21_decimal sub = DECIMAL_DEFAULT;
  if (s21_check_sub(num1, num2, &sub) == 0) {
    s21_super_decimal s_num1 = s21_convert_decimal_to_superdecimal(num1);
    s21_super_decimal s_num2 = s21_convert_decimal_to_superdecimal(num2);
    s21_normalisation_exponents(&s_num1, &s_num2);
    s21_super_decimal s_sub = s21_sub_with_check_signs(s_num1, s_num2);
    sub = s21_convert_superdecimal_to_decimal(s_sub);
    if (s21_check_decimal_to_zero(num1) == 0 &&
        s21_check_decimal_to_zero(num2) == 0) {
      int scale = s21_get_scale_decimal(num1) >= s21_get_scale_decimal(num2)
                      ? s21_get_scale_decimal(num1)
                      : s21_get_scale_decimal(num2);
      s21_set_scale_decimal(&sub, scale);
    }
  }
  return sub;
}

int s21_check_sub(s21_decimal num1, s21_decimal num2, s21_decimal *sub) {
  int error = 0;
  if (num1.value_type == s21_NAN || num2.value_type == s21_NAN ||
      (num1.value_type == s21_INFINITY && num2.value_type == s21_INFINITY) ||
      (num1.value_type == s21_NEGATIVE_INFINITY &&
       num2.value_type == s21_NEGATIVE_INFINITY)) {
    sub->value_type = s21_NAN;
    error = 1;
  } else if (num1.value_type == s21_INFINITY ||
             num2.value_type == s21_NEGATIVE_INFINITY) {
    *sub = s21_set_infinity_decimal(0);
    error = 1;
  } else if (num1.value_type == s21_NEGATIVE_INFINITY ||
             num2.value_type == s21_INFINITY) {
    *sub = s21_set_infinity_decimal(1);
    error = 1;
  }
  return error;
}

s21_super_decimal s21_sub_with_check_signs(s21_super_decimal s_num1,
                                           s21_super_decimal s_num2) {
  s21_super_decimal s_sub;
  int scale = s21_get_scale_super_decimal(s_num1);
  s21_set_scale_super_decimal(&s_num1, 0);
  s21_set_scale_super_decimal(&s_num2, 0);
  if ((s21_get_sign_super_decimal(s_num1) == 0 &&
       s21_get_sign_super_decimal(s_num2) == 1)) {
    s21_set_sign_super_decimal(&s_num2, 0);
    s_sub = s21_add_without_verify(s_num1, s_num2, 0);
  } else if ((s21_get_sign_super_decimal(s_num1) == 1 &&
              s21_get_sign_super_decimal(s_num2) == 0)) {
    s21_set_sign_super_decimal(&s_num1, 0);
    s_sub = s21_add_without_verify(s_num1, s_num2, 0);
    s21_set_sign_super_decimal(&s_sub, 1);
  } else if (s21_get_sign_super_decimal(s_num1) == 0 &&
             s21_get_sign_super_decimal(s_num2) == 0) {
    s_sub = s21_sub_without_verify(s_num1, s_num2);
  } else {
    s21_set_sign_super_decimal(&s_num1, 0);
    s21_set_sign_super_decimal(&s_num2, 0);
    s_sub = s21_sub_without_verify(s_num2, s_num1);
  }
  s21_set_scale_super_decimal(&s_sub, scale);
  return s_sub;
}

// вычитаем только 2 положительных числа
s21_super_decimal s21_sub_without_verify(s21_super_decimal s_num1,
                                         s21_super_decimal s_num2) {
  s21_super_decimal sub;
  // проверка какое число больше
  int flag = s21_is_greater_or_equal_super_decimal(s_num1, s_num2);
  s_num2 = s21_dop_code(s_num2);  //  перевод в допкод
  sub = s21_add_without_verify(s_num1, s_num2, 1);
  if (flag == 1) {
    sub = s21_dop_code(sub);  // если s_num2 переводим в доп-код результат
    s21_set_sign_super_decimal(&sub, 1);
  }
  return sub;
}

s21_super_decimal s21_dop_code(s21_super_decimal s_num) {
  for (int i = 0; i < 6; i++) {
    s_num.bits[i] = ~s_num.bits[i];
  }
  s21_super_decimal s_one = s21_init_int_super_decimal(1);
  s_num = s21_add_without_verify(s_num, s_one, 0);
  return s_num;
}

s21_decimal s21_mul(s21_decimal num1, s21_decimal num2) {
  s21_decimal mul = DECIMAL_DEFAULT;
  if (s21_check_mul(num1, num2, &mul) == 0) {
    s21_super_decimal s_num1 = s21_convert_decimal_to_superdecimal(num1);
    s21_super_decimal s_num2 = s21_convert_decimal_to_superdecimal(num2);
    s21_super_decimal s_mul = s21_mul_with_check_signs(s_num1, s_num2);
    mul = s21_convert_superdecimal_to_decimal(s_mul);
    if (s21_check_decimal_to_zero(mul) == 0 &&
        ((s21_check_decimal_to_zero(num1) == 0 &&
          s21_get_scale_decimal(num1) != 0) ||
         (s21_check_decimal_to_zero(num2) == 0 &&
          s21_get_scale_decimal(num2) != 0))) {
      int scale = s21_get_scale_decimal(num1) + s21_get_scale_decimal(num2);
      if (scale > 28) scale = 28;
      s21_set_scale_decimal(&mul, scale);
    }
  }
  return mul;
}

s21_super_decimal s21_mul_with_check_signs(s21_super_decimal s_num1,
                                           s21_super_decimal s_num2) {
  s21_super_decimal s_mul;
  int sign = 0, scale = s21_get_scale_super_decimal(s_num1) +
                        s21_get_scale_super_decimal(s_num2);
  s21_set_scale_super_decimal(&s_num1, 0);
  s21_set_scale_super_decimal(&s_num2, 0);
  int sign1 = s21_get_sign_super_decimal(s_num1);
  int sign2 = s21_get_sign_super_decimal(s_num2);
  if (sign1 == 1 && sign2 == 1) {
    s21_set_sign_super_decimal(&s_num1, 0);
    s21_set_sign_super_decimal(&s_num2, 0);
  } else if (sign1 == 0 && sign2 == 1) {
    s21_set_sign_super_decimal(&s_num2, 0);
    sign = 1;
  } else if (sign1 == 1 && sign2 == 0) {
    s21_set_sign_super_decimal(&s_num1, 0);
    sign = 1;
  }
  s_mul = s21_mul_without_verify(s_num2, s_num1);
  if (s_mul.value_type != s21_NORMAL_VALUE) {
    s_mul = s21_set_infinity_super_decimal(sign);
  } else {
    s21_set_sign_super_decimal(&s_mul, sign);
    s21_set_scale_super_decimal(&s_mul, scale);
  }
  return s_mul;
}

int s21_check_mul(s21_decimal num1, s21_decimal num2, s21_decimal *mul) {
  int error = 0;
  if (num1.value_type == s21_NAN || num2.value_type == s21_NAN ||
      (num1.value_type == s21_INFINITY &&
       num2.value_type == s21_NEGATIVE_INFINITY) ||
      (num1.value_type == s21_NEGATIVE_INFINITY &&
       num2.value_type == s21_INFINITY)) {
    mul->value_type = s21_NAN;
    error = 1;
  } else if (num1.value_type == s21_INFINITY ||
             num2.value_type == s21_INFINITY) {
    *mul = s21_set_infinity_decimal(0);
    error = 1;
  } else if (num1.value_type == s21_NEGATIVE_INFINITY ||
             num2.value_type == s21_NEGATIVE_INFINITY) {
    *mul = s21_set_infinity_decimal(1);
    error = 1;
  }
  return error;
}

// умножаются только два положительных числа
s21_super_decimal s21_mul_without_verify(s21_super_decimal s_num1,
                                         s21_super_decimal s_num2) {
  s21_super_decimal s_mul = SUPER_DECIMAL_DEFAULT;
  for (int i = 0; i < 192; i++) {
    s21_super_decimal s_buf = s21_shift_to_left_super_decimal(s_num1, i);
    if (s21_get_bit_super_decimal(s_num2, i) == 1) {
      s_mul = s21_add_without_verify(s_mul, s_buf, 0);
    }
  }
  return s_mul;
}

s21_decimal s21_div(s21_decimal num1, s21_decimal num2) {
  s21_decimal div = DECIMAL_DEFAULT;
  if (num1.value_type == s21_NAN || num2.value_type == s21_NAN ||
      (s21_check_decimal_to_zero(num1) == 0 &&
       s21_check_decimal_to_zero(num2) == 0) ||
      ((num1.value_type == s21_INFINITY ||
        num1.value_type == s21_NEGATIVE_INFINITY) &&
       (num2.value_type == s21_INFINITY ||
        num2.value_type == s21_NEGATIVE_INFINITY))) {
    div.value_type = s21_NAN;
  } else if (num1.value_type == s21_NORMAL_VALUE &&
             num2.value_type == s21_INFINITY) {
    s21_set_sign_decimal(&div, s21_get_sign_decimal(num1));
  } else if (num1.value_type == s21_NORMAL_VALUE &&
             num2.value_type == s21_NEGATIVE_INFINITY) {
    s21_set_sign_decimal(&div, !s21_get_sign_decimal(num1));
  } else if ((num1.value_type == s21_INFINITY ||
              num1.value_type == s21_NEGATIVE_INFINITY) &&
             num2.value_type == s21_NORMAL_VALUE) {
    int sign = num1.value_type == s21_INFINITY ? 0 : 1;
    div = s21_set_infinity_decimal(sign ^ s21_get_sign_decimal(num2));
  } else if (s21_check_decimal_to_zero(num2) == 0) {
    div = s21_set_infinity_decimal(s21_get_sign_decimal(num1) ^
                                   s21_get_sign_decimal(num2));
  } else if (s21_check_decimal_to_zero(num1) == 0 &&
             s21_get_scale_decimal(num1) != 0) {
    int scale = s21_get_scale_decimal(num1) + s21_get_scale_decimal(num2);
    if (scale > 28) scale = 28;
    s21_set_scale_decimal(&div, scale);
    s21_set_sign_decimal(
        &div, s21_get_sign_decimal(num1) ^ s21_get_sign_decimal(num2));
  } else {
    s21_super_decimal s_new1 = s21_convert_decimal_to_superdecimal(num1);
    s21_super_decimal s_new2 = s21_convert_decimal_to_superdecimal(num2);
    if (s21_get_scale_decimal(num1) != s21_get_scale_decimal(num2)) {
      s21_normalisation_exponents(&s_new1, &s_new2);
    }
    s21_super_decimal s_div = s21_div_without_verify(s_new1, s_new2);
    div = s21_convert_superdecimal_to_decimal(s_div);
    s21_decimal mod = s21_mod_ten_decimal(div);
    while (s21_check_decimal_to_zero(mod) == 0 &&
           s21_check_decimal_to_zero(div) != 0 &&
           s21_get_scale_decimal(div) > 0 && s21_get_scale_decimal(div) <= 28) {
      div = s21_div_by_10_decimal(div);
      mod = s21_mod_ten_decimal(div);
    }

    // tyt okruglyator
    s21_set_sign_decimal(
        &div, s21_get_sign_decimal(num1) ^ s21_get_sign_decimal(num2));
    if (div.value_type == s21_INFINITY && s21_get_sign_decimal(div) == 1) {
      div.value_type = s21_NEGATIVE_INFINITY;
      s21_set_sign_decimal(&div, 0);
    }
  }
  return div;
}

s21_super_decimal s21_div_without_verify(s21_super_decimal num1,
                                         s21_super_decimal num2) {
  s21_super_decimal rezult = SUPER_DECIMAL_DEFAULT;
  s21_super_decimal dividend = SUPER_DECIMAL_DEFAULT;
  s21_super_decimal zero = SUPER_DECIMAL_DEFAULT;
  s21_super_decimal one = s21_init_int_super_decimal(1);
  s21_set_scale_super_decimal(&num1, 0);
  s21_set_scale_super_decimal(&num2, 0);
  s21_set_sign_super_decimal(&num1, 0);
  s21_set_sign_super_decimal(&num2, 0);
  int shifts = 0, flag_start = 0, scale_rez = 0;
  for (int i = 191; i >= 0; i--) {
    dividend = s21_shift_to_left_super_decimal(dividend, 1);
    if (s21_get_bit_super_decimal(num1, i) == 1) {
      dividend = s21_add_without_verify(dividend, one, 0);
    }
    if (s21_is_greater_or_equal_super_decimal(dividend, num2) == 0) {
      if (flag_start == 1) rezult = s21_shift_to_left_super_decimal(rezult, 1);
      s21_set_bit_super_decimal(&rezult, 0);
      dividend = s21_sub_without_verify(dividend, num2);
      if (flag_start == 0) flag_start = 1;
    } else if (s21_is_greater_or_equal_super_decimal(dividend, num2) == 1 &&
               flag_start == 1) {
      rezult = s21_shift_to_left_super_decimal(rezult, 1);
    }
    if (flag_start == 1) shifts = shifts + 1;
  }
  int flag_check_infiniti = 0;
  if (s21_check_higher_bites_super_decimal(rezult) == 1)
    flag_check_infiniti = 1;

  while (s21_bit_equal_super_decimal(dividend, zero) != 0 && scale_rez < 29 &&
         rezult.bits[S_HIGHS] < 2147483647 && flag_check_infiniti == 0) {
    while (s21_is_greater_or_equal_super_decimal(dividend, num2) == 1 &&
           scale_rez < 29) {
      dividend = s21_mul_by_10_super_decimal(dividend);
      scale_rez++;
      rezult = s21_mul_by_10_super_decimal(rezult);
    }
    int count_sub = 0;
    while (s21_is_greater_or_equal_super_decimal(dividend, num2) == 0) {
      dividend = s21_sub_without_verify(dividend, num2);
      count_sub++;
    }
    rezult = s21_add_without_verify(rezult,
                                    (s21_init_int_super_decimal(count_sub)), 0);
  }
  if (flag_check_infiniti == 0) s21_set_scale_super_decimal(&rezult, scale_rez);
  return rezult;
}

void s21_normalisation_exponents(s21_super_decimal *s_num1,
                                 s21_super_decimal *s_num2) {
  int scale_norm = s21_get_scale_super_decimal(*s_num1) -
                   s21_get_scale_super_decimal(*s_num2);
  if (scale_norm > 0) {
    *s_num2 = s21_norm_decimal_in_super_decimal(*s_num2, scale_norm);
  } else if (scale_norm < 0) {
    *s_num1 = s21_norm_decimal_in_super_decimal(*s_num1, -1 * scale_norm);
  }
}

s21_super_decimal s21_norm_decimal_in_super_decimal(s21_super_decimal s_num,
                                                    int scale_norm) {
  s21_super_decimal s_res = s_num;
  for (int scale = scale_norm; scale > 0; scale--) {
    s_res = s21_mul_by_10_super_decimal(s_res);
  }
  s21_set_scale_super_decimal(&s_res,
                              scale_norm + s21_get_scale_super_decimal(s_res));
  return s_res;
}

s21_super_decimal s21_convert_decimal_to_superdecimal(s21_decimal num) {
  s21_super_decimal s_num;
  s_num.value_type = num.value_type;
  if (s_num.value_type == s21_NORMAL_VALUE) {
    s_num.bits[LOWS] = num.bits[LOW];
    s_num.bits[MIDS] = num.bits[MID];
    s_num.bits[HIGHS] = num.bits[HIGH];
    s_num.bits[S_LOWS] = 0;
    s_num.bits[S_MIDS] = 0;
    s_num.bits[S_HIGHS] = 0;
    s_num.bits[S_SCALE] = num.bits[SCALE];
  } else {
    s_num = s21_set_zero_super_decimal(s_num);
  }
  return s_num;
}

s21_decimal s21_convert_superdecimal_to_decimal(s21_super_decimal s_num) {
  s21_decimal num = DECIMAL_DEFAULT;
  if (s_num.value_type != s21_NORMAL_VALUE) {
    num.value_type = s_num.value_type;
  } else {
    s21_super_decimal s_mod = SUPER_DECIMAL_DEFAULT;
    int scale = s21_get_scale_super_decimal(s_num);
    int flag_zero_all_s_num = s21_check_super_decimal_to_zero(s_num);
    for (; scale > 28; scale--) {
      s_mod = s21_mod_ten_super_decimal(s_num);
      s_num = s21_div_by_10_super_decimal(s_num);
    }
    int flag_zero_higher_bites = s21_check_higher_bites_super_decimal(s_num);
    for (; scale > 0 &&
           (flag_zero_all_s_num == 0 || flag_zero_higher_bites == 1);) {
      s_mod = s21_mod_ten_super_decimal(s_num);
      s_num = s21_div_by_10_super_decimal(s_num);
      scale--;
      flag_zero_higher_bites = s21_check_higher_bites_super_decimal(s_num);
    }
    if (flag_zero_higher_bites == 0) {
      s21_super_decimal s_one = s21_init_int_super_decimal(1);
      s21_set_scale_super_decimal(&s_one, scale);
      s21_decimal mod = s21_copy_superdecimal_to_decimal(s_mod);
      int mod_int = 0;
      int flag_error = s21_from_decimal_to_int(mod, &mod_int);
      if (flag_error == 0 && mod_int >= 5 && mod_int <= 9) {
        if (s21_get_sign_super_decimal(s_num) == 0) {
          s_num = s21_add_with_check_signs(s_num, s_one);
        } else {
          s_num = s21_sub_with_check_signs(s_num, s_one);
        }
      }
      num = s21_copy_superdecimal_to_decimal(s_num);
    } else {  // num уже равен нулю
      num = s21_set_infinity_decimal(s21_get_sign_super_decimal(s_num));
    }
  }
  return num;
}

s21_decimal s21_floor(s21_decimal num) {
  s21_decimal res = DECIMAL_DEFAULT;
  if (res.value_type == s21_NORMAL_VALUE) {
    int sign = s21_get_sign_decimal(num);
    res = s21_truncate(num);
    if (sign == 1 && s21_is_not_equal(num, res) == 0) {
      s21_decimal one = s21_init_int_decimal(1);
      res = s21_sub(res, one);
    }
  } else {
    res = s21_set_zero_decimal();
  }
  return res;
}

// 0: старшие bits равны 0
int s21_check_higher_bites_super_decimal(s21_super_decimal s_num) {
  int flag_zero_higher_bites = 1;
  if (!s_num.bits[S_LOWS] && !s_num.bits[S_MIDS] && !s_num.bits[S_HIGHS]) {
    flag_zero_higher_bites = 0;
  }
  return flag_zero_higher_bites;
}

s21_decimal s21_copy_superdecimal_to_decimal(s21_super_decimal s_num) {
  s21_decimal num = DECIMAL_DEFAULT;
  num.value_type = s_num.value_type;
  if (num.value_type == s21_NORMAL_VALUE) {
    num.bits[LOW] = s_num.bits[LOWS];
    num.bits[MID] = s_num.bits[MIDS];
    num.bits[HIGH] = s_num.bits[HIGHS];
    num.bits[SCALE] = s_num.bits[S_SCALE];
  }
  return num;
}

s21_super_decimal s21_mul_by_10_super_decimal(s21_super_decimal s_num) {
  s21_super_decimal s_mul = SUPER_DECIMAL_DEFAULT;
  s_mul.bits[S_SCALE] = s_num.bits[S_SCALE];
  for (int i = 1; i <= 2; i++) {
    // 10 (decimal) == 1010 (binary)
    // смещаем влево на 1 бит, а потом ещё на 2
    s_num = s21_shift_to_left_super_decimal(s_num, i);
    s_mul = s21_add_with_check_signs(s_mul, s_num);
  }
  return s_mul;
}

s21_decimal s21_mod(s21_decimal num1, s21_decimal num2) {
  s21_decimal mod = DECIMAL_DEFAULT;
  if (num1.value_type == s21_NAN || num2.value_type == s21_NAN ||
      num1.value_type == s21_INFINITY ||
      num1.value_type == s21_NEGATIVE_INFINITY ||
      (num2.value_type == s21_NORMAL_VALUE &&
       s21_check_decimal_to_zero(num2) == 0)) {
    mod.value_type = s21_NAN;
  } else if (num2.value_type == s21_INFINITY ||
             num2.value_type == s21_NEGATIVE_INFINITY) {
    mod = num1;
  } else {
    if (s21_check_decimal_to_zero(num1) == 1 &&
        s21_check_decimal_to_zero(num2) == 1) {
      int sign1 = s21_get_sign_decimal(num1);
      int sign2 = s21_get_sign_decimal(num2);
      s21_set_sign_decimal(&num1, 0);
      s21_set_sign_decimal(&num2, 0);
      if (s21_is_greater(num1, num2) == 0) {
        s21_set_sign_decimal(&num1, sign1);
        s21_set_sign_decimal(&num2, sign2);
        s21_super_decimal s_num1 = s21_convert_decimal_to_superdecimal(num1);
        s21_super_decimal s_num2 = s21_convert_decimal_to_superdecimal(num2);
        s21_normalisation_exponents(&s_num1, &s_num2);
        s21_super_decimal s_res = s21_mod_without_verify(s_num1, s_num2);
        if (s21_check_super_decimal_to_zero(s_res) == 0) {
          s21_set_sign_decimal(&mod, sign1);
          s21_set_scale_decimal(&mod, s21_get_scale_super_decimal(s_res));
        } else {
          mod = s21_convert_superdecimal_to_decimal(s_res);
        }
      } else if (s21_is_less(num1, num2) == 0) {
        s21_set_sign_decimal(&num1, sign1);
        mod = num1;
      } else {
        int scale = s21_get_scale_decimal(num1);
        if (scale > 28) scale = 28;
        s21_set_scale_decimal(&mod, scale);
        s21_set_sign_decimal(&mod, sign1);
      }
    }
  }
  return mod;
}

s21_super_decimal s21_mod_without_verify(s21_super_decimal s_num1,
                                         s21_super_decimal s_num2) {
  s21_super_decimal s_mod;
  s21_super_decimal quotient;
  int sign1 = s21_get_sign_super_decimal(s_num1);
  int sign2 = s21_get_sign_super_decimal(s_num2);
  int sign = sign1 ^ sign2;
  quotient = s21_div_without_verify(s_num1, s_num2);
  s21_set_sign_super_decimal(&quotient, sign);
  quotient = s21_truncate_super_decimal(quotient);
  s_num2 = s21_mul_with_check_signs(quotient, s_num2);
  s_mod = s21_sub_with_check_signs(s_num1, s_num2);
  return s_mod;
}

s21_decimal s21_mod_ten_decimal(s21_decimal num) {
  s21_decimal mod;
  s21_decimal quotient;
  quotient = s21_div_by_10_decimal(num);
  s21_set_scale_decimal(&quotient, 0);
  s21_set_scale_decimal(&num, 0);
  s21_set_sign_decimal(&quotient, 0);
  s21_set_sign_decimal(&num, 0);
  quotient = s21_mul_by_10_decimal(quotient);
  mod = s21_sub(num, quotient);
  return mod;
}

s21_decimal s21_mul_by_10_decimal(s21_decimal num) {
  s21_decimal mul = DECIMAL_DEFAULT;
  mul.bits[SCALE] = num.bits[SCALE];
  for (int i = 1; i <= 2; i++) {
    // 10 (decimal) == 1010 (binary)
    // смещаем влево на 1 бит, а потом ещё на 2
    num = s21_shift_to_left_decimal(num, i);
    mul = s21_add(mul, num);
  }
  return mul;
}

s21_super_decimal s21_mod_ten_super_decimal(s21_super_decimal s_num) {
  s21_super_decimal s_mod;
  s21_super_decimal quotient;
  quotient = s21_div_by_10_super_decimal(s_num);
  quotient = s21_mul_by_10_super_decimal(quotient);
  s_mod = s21_sub_without_verify(s_num, quotient);
  return s_mod;
}

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  int error = 0;
  *dst = s21_init_decimal();
  int sign = src < 0 ? 1 : 0;
  s21_set_sign_decimal(dst, sign);
  if (src < 0) src = src * (-1);
  dst->bits[LOW] = src;
  return error;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  int flag = 0;
  s21_decimal new = s21_truncate(src);
  for (int i = 31; i < 96 && !flag; i++) {
    if (s21_get_bit_decimal(new, i) == 1) flag = 1;
  }
  if (flag == 0 && new.value_type == s21_NORMAL_VALUE) {
    *dst = (s21_get_sign_decimal(src)) == 1 ? (new.bits[LOW] * (-1))
                                            : new.bits[LOW];
  } else {
    flag = 1;
  }
  return flag;
}

// Отбрасываем дробную часть, приводя SCALE к 0
s21_decimal s21_truncate(s21_decimal num) {
  s21_decimal res = num;
  if (num.value_type == s21_NORMAL_VALUE) {
    int scale = s21_get_scale_decimal(num);
    if (scale > 0 && scale <= 28) {
      for (; scale > 0; scale--) {
        res = s21_div_by_10_decimal(res);
      }
      s21_set_sign_decimal(&res, s21_get_sign_decimal(num));
    }
    s21_set_sign_decimal(&res, s21_get_sign_decimal(num));
  } else if (num.value_type == s21_INFINITY) {
    res = s21_set_infinity_decimal(0);
  } else if (num.value_type == s21_NAN) {
    res = s21_init_int_decimal(0);
    res.value_type = s21_NAN;
  } else if (num.value_type == s21_NEGATIVE_INFINITY) {
    res = s21_set_infinity_decimal(1);
  }
  return res;
}

// Отбрасываем дробную часть, приводя SCALE к 0
s21_super_decimal s21_truncate_super_decimal(s21_super_decimal s_num) {
  s21_super_decimal s_res = s_num;
  if (s_num.value_type == s21_NORMAL_VALUE) {
    int scale = s21_get_scale_super_decimal(s_num);
    for (; scale > 0; scale--) {
      s_res = s21_div_by_10_super_decimal(s_res);
    }
    s21_set_sign_super_decimal(&s_res, s21_get_sign_super_decimal(s_num));
  } else {
    s_res = s21_set_zero_super_decimal();
  }
  return s_res;
}

// Делим децимал на 10 (уменьшая при этом SCALE на 1)
s21_decimal s21_div_by_10_decimal(s21_decimal num) {
  s21_decimal r = DECIMAL_DEFAULT;
  s21_decimal s = DECIMAL_DEFAULT;
  int a = 0, j = 0, flag_s = 0, count = 0;
  for (int i = 95; i >= 0; i--) {
    a <<= 1;
    a += s21_get_bit_decimal(num, i);
    if (a >= 10) {
      s21_set_bit_decimal(&r, j);
      a -= 10;
      if (!flag_s) flag_s = 1;
    }
    if (flag_s) j++;
  }
  for (int i = j - 1; i >= 0; i--, count++) {
    s21_init_bit_decimal(&s, count, s21_get_bit_decimal(r, i));
  }
  int scale = s21_get_scale_decimal(num);
  if (scale > 0) {
    s21_set_scale_decimal(&s, scale - 1);
  }
  return s;
}

// Делим супер децимал на 10 (уменьшая при этом SCALE на 1)
s21_super_decimal s21_div_by_10_super_decimal(s21_super_decimal s_num) {
  s21_super_decimal r = SUPER_DECIMAL_DEFAULT;
  s21_super_decimal s = SUPER_DECIMAL_DEFAULT;
  int a = 0, j = 0, flag_s = 0, count = 0;
  for (int i = 191; i >= 0; i--) {
    a <<= 1;
    a += s21_get_bit_super_decimal(s_num, i);
    if (a >= 10) {
      s21_set_bit_super_decimal(&r, j);
      a -= 10;
      if (!flag_s) flag_s = 1;
    }
    if (flag_s) j++;
  }
  for (int i = j - 1; i >= 0; i--, count++) {
    s21_init_bit_super_decimal(&s, count, s21_get_bit_super_decimal(r, i));
  }
  int scale = s21_get_scale_super_decimal(s_num);
  if (scale > 0) {
    s21_set_scale_super_decimal(&s, scale - 1);
  }
  s21_set_sign_super_decimal(&s, s21_get_sign_super_decimal(s_num));
  return s;
}

int s21_is_greater(s21_decimal num1, s21_decimal num2) {
  //  находим больший децимал, оба числа приводим к большему скейлу
  //  и сохраняем в супердецимал, которые и сравниваем
  int flag = 1;
  if ((num1.value_type == s21_INFINITY && num2.value_type == s21_INFINITY) ||
      (num1.value_type == s21_NEGATIVE_INFINITY &&
       num2.value_type == s21_NEGATIVE_INFINITY) ||
      (num1.value_type == s21_NAN || num2.value_type == s21_NAN)) {
    flag = 1;
  } else if ((num1.value_type != s21_NEGATIVE_INFINITY &&
              num2.value_type == s21_NEGATIVE_INFINITY) ||
             (num1.value_type == s21_INFINITY &&
              num2.value_type != s21_INFINITY)) {
    flag = 0;
  } else if ((num1.value_type != s21_INFINITY &&
              num2.value_type == s21_INFINITY) ||
             (num1.value_type == s21_NEGATIVE_INFINITY &&
              num2.value_type != s21_NEGATIVE_INFINITY)) {
    flag = 1;
  } else if (s21_get_sign_decimal(num1) < s21_get_sign_decimal(num2)) {
    flag = 0;
  } else if (s21_get_sign_decimal(num1) > s21_get_sign_decimal(num2)) {
    flag = 1;
  } else if (s21_get_scale_decimal(num1) == s21_get_scale_decimal(num2)) {
    flag = s21_bit_greater_decimal(num1, num2);
    if (s21_get_sign_decimal(num1) == 1 && s21_get_sign_decimal(num2) == 1 &&
        s21_is_not_equal(num1, num2) == 0)
      flag = !flag;
  } else {
    s21_super_decimal s_num1 = s21_convert_decimal_to_superdecimal(num1);
    s21_super_decimal s_num2 = s21_convert_decimal_to_superdecimal(num2);
    s21_normalisation_exponents(&s_num1, &s_num2);
    flag = s21_bit_greater_super_decimal(s_num1, s_num2);
    if (s21_get_sign_decimal(num1) == 1 && s21_get_sign_decimal(num2) == 1 &&
        s21_is_not_equal(num1, num2) == 0)
      flag = !flag;
  }
  return flag;
}

int s21_is_greater_or_equal(s21_decimal num1, s21_decimal num2) {
  //  если децимал 1 больше или равен децималу 2 возращаем 0, иначе 1
  int flag = 1;
  if (s21_is_greater(num1, num2) == 0 || s21_is_equal(num1, num2) == 0) {
    flag = 0;
  }
  return flag;
}

int s21_is_less(s21_decimal num1, s21_decimal num2) {
  int flag = 1;
  if ((num1.value_type == s21_INFINITY && num2.value_type == s21_INFINITY) ||
      (num1.value_type == s21_NEGATIVE_INFINITY &&
       num2.value_type == s21_NEGATIVE_INFINITY) ||
      (num1.value_type == s21_NAN || num2.value_type == s21_NAN)) {
    flag = 1;
  } else {
    flag = !s21_is_greater_or_equal(num1, num2);
  }
  return flag;
}

int s21_is_less_or_equal(s21_decimal num1, s21_decimal num2) {
  int flag = 1;
  if (s21_is_less(num1, num2) == 0 || s21_is_equal(num1, num2) == 0) {
    flag = 0;
  }
  return flag;
}

// 0 - это больше или равно, 1 - просто меньше
int s21_is_greater_or_equal_super_decimal(s21_super_decimal num1,
                                          s21_super_decimal num2) {
  int flag = 1;
  if ((s21_bit_equal_super_decimal(num1, num2) == 0) ||
      (s21_bit_greater_super_decimal(num1, num2) == 0)) {
    flag = 0;
  }
  return flag;
}

// Побитовое сравнение децимал. Если a > b возвращаем 0, иначе 1
int s21_bit_greater_decimal(s21_decimal a, s21_decimal b) {
  int flag = 1;
  int check = 0;
  for (int j = HIGH; j >= LOW && !check; j--) {
    if ((unsigned)a.bits[j] > (unsigned)b.bits[j]) {
      flag = 0;
      check = 1;
    } else if ((unsigned)a.bits[j] < (unsigned)b.bits[j]) {
      check = 1;
    }
  }
  return flag;
}

int s21_bit_greater_super_decimal(s21_super_decimal a, s21_super_decimal b) {
  int flag = 1;
  int check = 0;
  for (int j = S_HIGHS; j >= LOWS && !check; j--) {
    if ((unsigned)a.bits[j] > (unsigned)b.bits[j]) {
      flag = 0;
      check = 1;
    } else if ((unsigned)a.bits[j] < (unsigned)b.bits[j]) {
      check = 1;
    }
  }
  return flag;
}

int s21_bit_equal_super_decimal(s21_super_decimal a, s21_super_decimal b) {
  int flag = 0;
  for (int j = S_HIGHS; j >= LOWS && !flag; j--) {
    if (a.bits[j] != b.bits[j]) {
      flag = 1;
    }
  }
  return flag;
}

int s21_is_equal(s21_decimal num1, s21_decimal num2) {
  int answer = 0;
  if ((num1.value_type == s21_INFINITY && num2.value_type == s21_INFINITY) ||
      (num1.value_type == s21_NEGATIVE_INFINITY &&
       num2.value_type == s21_NEGATIVE_INFINITY)) {
    answer = 0;
  } else if (num1.value_type == s21_INFINITY ||
             num2.value_type == s21_INFINITY ||
             num1.value_type == s21_NEGATIVE_INFINITY ||
             num2.value_type == s21_NEGATIVE_INFINITY ||
             num1.value_type == s21_NAN || num2.value_type == s21_NAN) {
    answer = 1;
  } else if (s21_check_decimal_to_zero(num1) == 0 &&
             s21_check_decimal_to_zero(num2) == 0) {
    answer = 0;
  } else if (s21_get_sign_decimal(num1) != s21_get_sign_decimal(num2)) {
    answer = 1;
  } else {
    s21_super_decimal s_num1 = s21_convert_decimal_to_superdecimal(num1);
    s21_super_decimal s_num2 = s21_convert_decimal_to_superdecimal(num2);
    s21_normalisation_exponents(&s_num1, &s_num2);
    answer = s21_bit_equal_super_decimal(s_num1, s_num2);
  }
  return answer;
}

int s21_is_not_equal(s21_decimal num1, s21_decimal num2) {
  return !s21_is_equal(num1, num2);
}

s21_decimal s21_negate(s21_decimal num) {
  s21_decimal res = num;
  if (res.value_type == s21_NORMAL_VALUE) {
    int sign = !((unsigned)num.bits[SCALE] >> 31);
    s21_set_sign_decimal(&res, sign);
  } else if (num.value_type == s21_INFINITY) {
    res = s21_set_infinity_decimal(0);
  } else if (num.value_type == s21_NAN) {
    res = s21_init_int_decimal(0);
    res.value_type = s21_NAN;
  } else if (num.value_type == s21_NEGATIVE_INFINITY) {
    res = s21_set_infinity_decimal(1);
  }
  return res;
}

s21_decimal s21_round(s21_decimal num) {
  s21_decimal res = num;
  int scale = s21_get_scale_decimal(res);
  if (res.value_type == s21_NORMAL_VALUE && scale > 0) {
    int sign = s21_get_sign_decimal(res);
    s21_set_sign_decimal(&res, 0);
    for (; scale > 1; scale--) {
      res = s21_div_by_10_decimal(res);
    }
    s21_decimal ten = s21_init_int_decimal(10);
    s21_set_scale_decimal(&res, 0);
    s21_decimal mod_dec = s21_mod(res, ten);
    s21_set_scale_decimal(&res, scale);
    int mod = 0;
    if (s21_from_decimal_to_int(mod_dec, &mod) == 0) {
      res = s21_div_by_10_decimal(res);
      if (mod >= 5 && mod <= 9) {
        s21_decimal one = s21_init_int_decimal(1);
        res = s21_add(res, one);
      }
    }
    s21_set_sign_decimal(&res, sign);
  } else if (res.value_type != s21_NORMAL_VALUE) {
    res = s21_init_decimal();
    res.value_type = num.value_type;
  }
  return res;
}
s21_decimal s21_init_int_decimal(int i) {
  s21_decimal s_one = DECIMAL_DEFAULT;
  s_one.bits[LOW] = i;
  return s_one;
}

s21_super_decimal s21_init_int_super_decimal(int i) {
  s21_super_decimal s_one = SUPER_DECIMAL_DEFAULT;
  s_one.bits[LOWS] = i;
  return s_one;
}

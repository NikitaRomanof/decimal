#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  s21_NORMAL_VALUE = 0,
  s21_INFINITY = 1,
  s21_NEGATIVE_INFINITY = 2,
  s21_NAN = 3
} value_type_t;
enum { LOW, MID, HIGH, SCALE };  // DECIMAL ENUM
enum {
  LOWS,
  MIDS,
  HIGHS,
  S_LOWS,
  S_MIDS,
  S_HIGHS,
  S_SCALE
};  //  SUPER DECIMAL ENUM

typedef struct {
  int bits[4];
  value_type_t value_type;
} s21_decimal;
#define DECIMAL_DEFAULT {{0}, s21_NORMAL_VALUE};

typedef struct {
  int bits[7];
  value_type_t value_type;
} s21_super_decimal;
#define SUPER_DECIMAL_DEFAULT {{0}, s21_NORMAL_VALUE};

s21_decimal s21_add(s21_decimal num1, s21_decimal num2);
s21_decimal s21_sub(s21_decimal num1, s21_decimal num2);
s21_decimal s21_mul(s21_decimal num1, s21_decimal num2);
s21_decimal s21_div(s21_decimal num1, s21_decimal num2);
s21_decimal s21_mod(s21_decimal num1, s21_decimal num2);

int s21_is_less(s21_decimal num1, s21_decimal num2);
int s21_is_less_or_equal(s21_decimal num1, s21_decimal num2);
int s21_is_greater(s21_decimal num1, s21_decimal num2);
int s21_is_greater_or_equal(s21_decimal num1, s21_decimal num2);
int s21_is_equal(s21_decimal num1, s21_decimal num2);
int s21_is_not_equal(s21_decimal num1, s21_decimal num2);

int s21_is_greater_or_equal_super_decimal(s21_super_decimal num1,
                                          s21_super_decimal num2);

int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);

// Округляет указанное Decimal число до ближайшего целого числа
// в сторону отрицательной бесконечности
s21_decimal s21_floor(s21_decimal num);
// Округляет Decimal до ближайшего целого числа
s21_decimal s21_round(s21_decimal num);
s21_decimal s21_truncate(s21_decimal num);
s21_super_decimal s21_truncate_super_decimal(s21_super_decimal s_num);
// Изменяет знак decimal на противоположный
s21_decimal s21_negate(s21_decimal num);

// Узнает бит pos (0-95 бит) в decimal
int s21_get_bit_decimal(s21_decimal num, int pos);
// Устанавливает бит (0-95 бит) в decimal
void s21_set_bit_decimal(s21_decimal *num, int pos);
// Сбрасывает бит pos (0-95 бит) в decimal
void s21_clear_bit_decimal(s21_decimal *num, int pos);
// Устанавливает значение bit в позицию pos (0-95 бит) (в decimal)
void s21_init_bit_decimal(s21_decimal *num, int pos, int bit);

// Узнает знак в decimal: 0 - положительный, 1 - отрицательный
int s21_get_sign_decimal(s21_decimal num);
// Устанавливает знак в decimal: 0 - положительный, 1 - отрицательный
void s21_set_sign_decimal(s21_decimal *num, int sign);

int s21_get_scale_decimal(s21_decimal num);
void s21_set_scale_decimal(s21_decimal *num, int decimalPow);

int s21_get_sign_from_float(int num);
int s21_get_exp_from_float(int num);
int s21_get_mantiss_from_float(int num);

// Обнуляет только число в decimal
s21_decimal s21_set_zero_decimal();
s21_super_decimal s21_set_zero_super_decimal();
// Обнуляет весь decimal
s21_decimal s21_init_decimal();
// Проверяет decimal на ноль: 0 - ноль, 1 - не ноль
int s21_check_decimal_to_zero(s21_decimal num);
// Проверяет super_decimal на ноль: 0 - ноль, 1 - не ноль
int s21_check_super_decimal_to_zero(s21_super_decimal s_num);

// dop_code = 1 если операция производится в доп.коде
s21_super_decimal s21_add_without_verify(s21_super_decimal num1,
                                         s21_super_decimal num2, int dop_code);
s21_super_decimal s21_sub_without_verify(s21_super_decimal s_num1,
                                         s21_super_decimal s_num2);
s21_super_decimal s21_mul_without_verify(s21_super_decimal num1,
                                         s21_super_decimal num2);
s21_super_decimal s21_div_without_verify(s21_super_decimal num1,
                                         s21_super_decimal num2);

s21_super_decimal s21_add_with_check_signs(s21_super_decimal s_num1,
                                           s21_super_decimal s_num2);
s21_super_decimal s21_sub_with_check_signs(s21_super_decimal s_num1,
                                           s21_super_decimal s_num2);
s21_super_decimal s21_mul_with_check_signs(s21_super_decimal s_num1,
                                           s21_super_decimal s_num2);
s21_super_decimal s21_mod_without_verify(s21_super_decimal s_num1,
                                         s21_super_decimal s_num2);
s21_super_decimal s21_mod_ten_super_decimal(s21_super_decimal s_num);
s21_super_decimal s21_dop_code(s21_super_decimal s_num);

// проверка обоих чисел на НЕчисло и бесконечность
int s21_check_add(s21_decimal num1, s21_decimal num2, s21_decimal *sum);
int s21_check_sub(s21_decimal num1, s21_decimal num2, s21_decimal *sub);
int s21_check_mul(s21_decimal num1, s21_decimal num2, s21_decimal *sum);

// Устанавливает infinity в decimal
s21_decimal s21_set_infinity_decimal(int sign);
s21_super_decimal s21_set_infinity_super_decimal(int sign);
// Смещает decimal на shift битов влево
s21_decimal s21_shift_to_left_decimal(s21_decimal num, int shift);
s21_super_decimal s21_shift_to_left_super_decimal(s21_super_decimal s_num,
                                                  int shift);
s21_super_decimal s21_shift_to_right_super_decimal(s21_super_decimal s_num);

s21_decimal s21_div_by_10_decimal(s21_decimal num);
s21_super_decimal s21_div_by_10_super_decimal(s21_super_decimal num);

// Побитово сравнивает а с b, без учета scale
int s21_bit_greater_decimal(s21_decimal a, s21_decimal b);
int s21_bit_greater_super_decimal(s21_super_decimal a, s21_super_decimal b);
int s21_bit_equal_super_decimal(s21_super_decimal a, s21_super_decimal b);

s21_super_decimal s21_norm_decimal_in_super_decimal(s21_super_decimal s_num,
                                                    int scale_norm);
s21_decimal s21_mul_by_10_decimal(s21_decimal num);
s21_super_decimal s21_mul_by_10_super_decimal(s21_super_decimal s_num);
s21_super_decimal s21_convert_decimal_to_superdecimal(s21_decimal num);
s21_decimal s21_convert_superdecimal_to_decimal(s21_super_decimal s_num);
int s21_check_higher_bites_super_decimal(s21_super_decimal s_num);

s21_decimal s21_copy_superdecimal_to_decimal(s21_super_decimal s_num);
void s21_normalisation_exponents(s21_super_decimal *s_num1,
                                 s21_super_decimal *s_num2);

int s21_get_sign_super_decimal(s21_super_decimal s_num);
void s21_set_sign_super_decimal(s21_super_decimal *s_num, int sign);
int s21_get_scale_super_decimal(s21_super_decimal s_num);
void s21_set_scale_super_decimal(s21_super_decimal *s_num, int decimalPow);

int s21_get_bit_super_decimal(s21_super_decimal s_num, int pos);
void s21_set_bit_super_decimal(s21_super_decimal *s_num, int pos);
void s21_clear_bit_super_decimal(s21_super_decimal *s_num, int pos);
void s21_init_bit_super_decimal(s21_super_decimal *s_num, int pos, int bit);

s21_decimal s21_init_int_decimal(int i);
s21_super_decimal s21_init_int_super_decimal(int i);

s21_super_decimal s21_increase_super_decimal_to_max_scale(
    s21_super_decimal s_num);
s21_super_decimal s21_copy_superdecimal_with_S_SCALE_0(s21_super_decimal s_num);
s21_super_decimal s21_init_max_value_div_10_super_decimal();

s21_decimal s21_mod_ten_decimal(s21_decimal num);

#endif  // SRC_S21_DECIMAL_H_

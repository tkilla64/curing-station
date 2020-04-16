//
// pwmtables.c - pwm tables

#include <Arduino.h>

#define TABLE_SIZE 50

static int table_scale = 1;

static const uint8_t dcmotor_table[TABLE_SIZE] =
{
  0,  2,  4,  6,  8, 10, 12, 14, 16, 18,
 20, 25, 30, 35, 40, 45, 50, 55, 60, 65,
 70, 75, 80, 85, 90, 95,100,105,110,110,
 120,130,140,150,160,170,180,190,200,205,
 210,215,220,225,230,235,240,245,250,255
};

static const uint8_t uvled_table[TABLE_SIZE] =
{
  0,  5,  10,  15,  20, 25, 30, 35, 40, 45,
 50, 55, 60, 65, 70, 75, 80, 85, 90, 95,
100,105,110,115,120,125,130,135,140,145,
150,155,160,165,170,175,180,185,190,195,
200,210,215,220,225,230,235,240,250,255
};

int getTableRange(void)
{
  return TABLE_SIZE * table_scale;
}

void setTableScale(int scale)
{
  table_scale = scale;
}

int getUVLedTable(int index)
{
  if (index > getTableRange())
    index = getTableRange();
  return uvled_table[index / table_scale];
}

int getDcMotorTable(int index)
{
  if (index > getTableRange())
    index = getTableRange();
  return dcmotor_table[index / table_scale];
}

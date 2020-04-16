//
// pwmtables.h - prototypes

#ifdef __cplusplus
extern "C" {
#endif

int getTableRange(void);

void setTableScale(int scale);

int getUVLedTable(int index);

int getDcMotorTable(int index);

#ifdef __cplusplus
}
#endif

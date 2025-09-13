#ifndef REPORT_H
#define REPORT_H

void reportMenu();
void salesReportByPeriod(const char *prefix, const char *label);
void productWiseReport();
void customerSpendingReport();
void totalIncomeByRange(const char *startDate, const char *endDate);

#endif

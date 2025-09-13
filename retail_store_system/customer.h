#ifndef CUSTOMER_H
#define CUSTOMER_H

typedef struct {
    int custID;
    char name[50];
    char phone[15];
    char email[50];
    char address[100];
} Customer;

void addCustomer();
void searchCustomer();
void updateCustomer();
void fetchReceiptHistory(int cid);

#endif

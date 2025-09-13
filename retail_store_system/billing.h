#ifndef BILLING_H
#define BILLING_H

typedef struct item {
    char name[50];
    int quantity;
    float price;
    float total;
    int product_code;
    struct item *next;
} item;

extern item *head;
extern int globalReceiptID;

void addItem();
void displayBill();
void finalizeBill(int custID);
void freeCart();
void print_bill_sidebar(int start_row);

#endif

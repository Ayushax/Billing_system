#include <stdio.h>
#include "billing.h"
#include "customer.h"
#include "report.h"
#include "utils.h"

void billingMenu() {
    int ch,id;
    while(1){
        clear_screen();
        print_left_header("Billing");
        print_bill_sidebar(2);
        printf("1. Add Item\n2. Display Bill\n3. Finalize Bill\n4. Back\nChoice: ");
        if (scanf("%d",&ch)!=1){ while(getchar()!='\n'); continue; }
        if(ch==1){ addItem(); getchar(); getchar(); }
        else if(ch==2){ displayBill(); getchar(); getchar(); }
        else if(ch==3){ printf("CustomerID: "); if (scanf("%d",&id)==1) finalizeBill(id); getchar(); getchar(); }
        else if(ch==4) break;
    }
}

void customerMenu() {
    int ch;
    while(1){
        clear_screen();
        print_left_header("Customer");
        printf("1. Add Customer\n2. Search Customer\n3. Update Customer\n4. Fetch Receipts\n5. Back\nChoice: ");
        if (scanf("%d",&ch)!=1){ while(getchar()!='\n'); continue; }
        if(ch==1){ addCustomer(); getchar(); getchar(); }
        else if(ch==2){ searchCustomer(); getchar(); getchar(); }
        else if(ch==3){ updateCustomer(); getchar(); getchar(); }
        else if(ch==4){ int id; printf("Enter ID: "); scanf("%d",&id); fetchReceiptHistory(id); getchar(); getchar(); }
        else if(ch==5) break;
    }
}

int main() {
    enable_ansi_if_windows();
    int ch;
    while(1){
        clear_screen();
        print_left_header("Main Menu");
        printf("1. Billing\n2. Customer Management\n3. Reports\n4. Exit\nChoice: ");
        if (scanf("%d",&ch)!=1){ while(getchar()!='\n'); continue; }
        if(ch==1) billingMenu();
        else if(ch==2) customerMenu();
        else if(ch==3) reportMenu();
        else if(ch==4) break;
    }
    clear_screen();
    return 0;
}

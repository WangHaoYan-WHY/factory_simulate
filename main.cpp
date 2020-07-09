#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <time.h>
using namespace std;
int Note_Num{0};// Use Note_Num to count the number of times which Buffer_State does not change
int Part_Count{0}, Product_Count{0}; // Count the number of completed parts and completed products
int Max_A{6}; // Set the capacity of A part
int Max_B{5}; // Set the capacity of B part
int Max_C{4}; // Set the capacity of C part
int Max_D{3}; // Set the capacity of D part
int Buffer_State[4]; //Represent four parts in Buffer_State
mutex buffer_m; // Mutex for the buffer
condition_variable cv1, cv2; // Condition variables used to notify part and product
const int m = 20, n = 16; // I set m and n as global variables so that we can change them
int * RandNum(int Min, int Max, int id); // Used seed id to generate the random number between Min and Max
void Notify(int Num){ // Each time call this function, the Note_Num will change
    if (Num == 1){
        Note_Num = Note_Num + 1;
    }
    if (Num == 2){
        Note_Num = 0;
    }
}
void Place(int A, int B, int C, int D, int id, int iteration){ // Place operation for PartWorker
    while(A != 0 || B != 0 || C != 0 || D != 0){ // judge whether this request is completed
        unique_lock<mutex> lck(buffer_m);
        while ((Buffer_State[0] == Max_A) && (Buffer_State[1] == Max_B)
               && (Buffer_State[2] == Max_C) && (Buffer_State[3] == Max_D)
               && (Product_Count != 16)){ // If all is max, then wait
            cv1.wait(lck);
        }
        int BS1{Buffer_State[0]}, BS2{Buffer_State[1]}, BS3{Buffer_State[2]}, BS4{Buffer_State[3]};
        cout << "Part Worker ID: " << id << endl;
        cout << "Iteration: " << iteration << endl;
        cout << "Buffer State: (" << Buffer_State[0] <<","
        << Buffer_State[1] << "," << Buffer_State[2] << ","
        << Buffer_State[3] << ")" << endl;
        cout << "Place Request: (" << A <<","
        << B << "," << C << ","
        << D << ")" << endl;
        if (A != 0 && (Buffer_State[0] + A) <= Max_A){ // Do the change to the Buffer_State
            Buffer_State[0] = Buffer_State[0] + A;
            A = 0;
        }
        else if (A != 0 && (Buffer_State[0] + A) > Max_A){
            A = A - (Max_A - Buffer_State[0]);
            Buffer_State[0] = Max_A;
        }
        if (B != 0 && (Buffer_State[1] + B) <= Max_B){
            Buffer_State[1] = Buffer_State[1] + B;
            B = 0;
        }
        else if (B != 0 && (Buffer_State[1] + B) > Max_B){
            B = B - (Max_B - Buffer_State[1]);
            Buffer_State[1] = Max_B;
        }
        if (C != 0 && (Buffer_State[2] + C) <= Max_C){
            Buffer_State[2] = Buffer_State[2] + C;
            C = 0;
        }
        else if (C != 0 && (Buffer_State[2] + C) > Max_C){
            C = C - (Max_C - Buffer_State[2]);
            Buffer_State[2] = Max_C;
        }
        if (D != 0 && (Buffer_State[3] + D) <= Max_D){
            Buffer_State[3] = Buffer_State[3] + D;
            D = 0;
        }
        else if (D != 0 && (Buffer_State[3] + D) > Max_D){
            D = D - (Max_D - Buffer_State[3]);
            Buffer_State[3] = Max_D;
        }
        cout << "Updated Buffer State: (" << Buffer_State[0] <<","
        << Buffer_State[1] << "," << Buffer_State[2] << ","
        << Buffer_State[3] << ")" << endl;
        cout << "Updated Place Request: (" << A <<","
        << B << "," << C << ","
        << D << ")" << endl;
        if (BS1 == Buffer_State[0] && BS2 == Buffer_State[1] && BS3 == Buffer_State[2] && BS4 == Buffer_State[3]){ //If not change Buffer_State, then call Notify function to increase the Note_Num
            Notify(1);
        }
        else{ // If change Buffer_State, then call Notify function to set the Note_Num as 0
            Notify(2);
        }
        if (Note_Num == 2000){// If the Note_Num has increased to 2000, then there can be deadlock
            // which is all the threads can not change the Buffer_State
            cout << "Deadlock Detected" << endl;
            cout << "Aborted Iteration: " << iteration << endl;
            cout << endl;
            Note_Num = 0;
            break;
        }
        cout << endl;
        if (Product_Count == n){ // If all the product threads are completed, then use CV1 to notify
            cv1.notify_one();
        }
        else{ // If there are still some product threads are processing, then use CV2 to notify
            cv2.notify_one();
        }
        lck.unlock();
    }
}
void Pick_Up (int A, int B, int C, int D, int id, int iteration){ // Pick_Up operation for ProductWorker
    while(A != 0 || B != 0 || C != 0 || D != 0){ // Judge whether the request is completed
        unique_lock<mutex> lck(buffer_m);
        while ((Buffer_State[0] == 0) &&
               (Buffer_State[1] == 0) &&
               (Buffer_State[2] == 0) &&
               (Buffer_State[3] == 0) &&
               (Part_Count != 20)){ // If there are no item to pick up, then wait
            cv2.wait(lck);
        }
        int BS1{Buffer_State[0]}, BS2{Buffer_State[1]}, BS3{Buffer_State[2]}, BS4{Buffer_State[3]};
        cout << "Product Worker ID: " << id << endl;
        cout << "Iteration: " << iteration << endl;
        cout << "Buffer State: (" << Buffer_State[0] <<","
        << Buffer_State[1] << "," << Buffer_State[2] << ","
        << Buffer_State[3] << ")" << endl;
        cout << "Pickup Request: (" << A <<","
        << B << "," << C << ","
        << D << ")" << endl;
        if (A != 0 && A <= Buffer_State[0]){ // Do the change to the Buffer_State
            Buffer_State[0] = Buffer_State[0] - A;
            A = 0;
        }
        else if (A != 0 && A > Buffer_State[0]){
            A = A - Buffer_State[0];
            Buffer_State[0] = 0;
        }
        if (B != 0 && B <= Buffer_State[1]){
            Buffer_State[1] = Buffer_State[1] - B;
            B = 0;
        }
        else if (B != 0 && B > Buffer_State[1]){
            B = B - Buffer_State[1];
            Buffer_State[1] = 0;
        }
        if (C != 0 && C <= Buffer_State[2]){
            Buffer_State[2] = Buffer_State[2] - C;
            B = 0;
        }
        else if (C != 0 && C > Buffer_State[2]){
            C = C - Buffer_State[2];
            Buffer_State[2] = 0;
        }
        if (D != 0 && D <= Buffer_State[3]){
            Buffer_State[3] = Buffer_State[3] - D;
            D = 0;
        }
        else if (D != 0 && D > Buffer_State[3]){
            D = D - Buffer_State[3];
            Buffer_State[3] = 0;
        }
        cout << "Updated Buffer State: (" << Buffer_State[0] <<","
        << Buffer_State[1] << "," << Buffer_State[2] << ","
        << Buffer_State[3] << ")" << endl;
        cout << "Updated Pickup Request: (" << A <<","
        << B << "," << C << ","
        << D << ")" << endl;
        if (BS1 == Buffer_State[0] && BS2 == Buffer_State[1] && BS3 == Buffer_State[2] && BS4 == Buffer_State[3]){ //If not change Buffer_State, then call Notify function to increase the Note_Num
            Notify(1);
        }
        else{ // If change Buffer_State, then call Notify function to set the Note_Num as 0
            Notify(2);
        }
        if (Note_Num == 2000){ // If the Note_Num has increased to 2000, then there can be deadlock
            // which is all the threads can not change the Buffer_State
            cout << "Deadlock Detected" << endl;
            cout << "Aborted Iteration: " << iteration << endl;
            cout << endl;
            Note_Num = 0;
            break;
        }
        cout << endl;
        if (Part_Count == m){ // If all the part threads are completed, then use CV2 to notify
            cv2.notify_one();
        }
        else{ // If there are still some part threads are processing, then use CV1 to notify
            cv1.notify_one();
        }
        lck.unlock();
    }
}
void PartWorker (int id){
    int iteration{1}, seed{id};
    while (iteration != 6){ // Do the iteration
        // Following is generating the random request
        srand (seed);
        seed = seed * 89;
        int part_list[4];
        for (int i = 0; i < 4; i++){
            part_list[i] = 0;
        }
        int num_part{4};
        int Num = 1 + rand() % (3);
        int *Num1 = RandNum(1, 5, id);
        for (int i = 0; i < Num; i++){
            if (i == Num - 1){
                part_list[Num1[i] - 1] = num_part;
            }
            else{
                int Num2;
                if(i == 0 && Num == 3){
                    Num2 = 1 + rand() % (num_part - 2);
                }
                else{
                    Num2 = 1 + rand() % (num_part - 1);
                }
                part_list[Num1[i] - 1] = Num2;
                num_part = num_part - Num2;
            }
        }
        // Call Place function to complete the request
        Place(part_list[0], part_list[1], part_list[2], part_list[3], id, iteration);
        iteration++;
    }
    Part_Count = Part_Count + 1;
}
void ProductWorker (int id){
    int iteration{1}, seed{id};
    while (iteration != 6){ // Do the iteration
        // Following is generating the random request
        srand (seed);
        seed = seed * 97;
        int product_list[4];
        for (int i = 0; i < 4; i++){
            product_list[i] = 0;
        }
        int num_product{5};
        int *Num1 = RandNum(1, 5, id);
        for (int i = 0; i < 3; i++){
            if (i == 2){
                product_list[Num1[i] - 1] = num_product;
            }
            else{
                int Num2;
                if(i == 0){
                    Num2 = 1 + rand() % (num_product - 2);
                }
                else{
                    Num2 = 1 + rand() % (num_product - 1);
                }
                product_list[Num1[i] - 1] = Num2;
                num_product = num_product - Num2;
            }
        }
        // Call Pick_Up function to complete the request
        Pick_Up(product_list[0], product_list[1], product_list[2], product_list[3], id, iteration);
        iteration++;
    }
    Product_Count = Product_Count + 1;
}
int * RandNum(int Min, int Max, int id){ // generate the random between Min and Max by using seed id
    int *t=new int[4];
    srand(id + 200);
    int i, j;
    for (i = 0; i < 4; i++)
    {
        t[i] = Min + rand() % (Max-Min);
        for (j = 0; j < i; j++)
        {
            if (t[i] == t[j])
            {
                i--;
                break;
            }
        }
    }
    return t;
}
int main(){
    for (int i = 0; i < 4; i++){
        Buffer_State[i] = 0;
    }
    // const int m = 20, n = 16; //m: number of Part Workers
    // I move this to the global variables so that we can change the number
    thread partW[m];
    //n: number of Product Workers //m>n
    thread prodW[n];
    for (int i = 0; i < n; i++){
        partW[i] = thread(PartWorker, i); prodW[i] = thread(ProductWorker, i);
    }
    for (int i = n; i<m; i++) {
        partW[i] = thread(PartWorker, i);
    }
    /* Join the threads to the main threads */
    for (int i = 0; i < n; i++) {
        partW[i].join(); prodW[i].join();
    }
    for (int i = n; i<m; i++) {
        partW[i].join();
    }
    cout << "Finish!" << endl;
    getchar(); getchar(); return 0;
}

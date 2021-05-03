#include<iostream>
#include<string>
#include "grid.cpp"

using namespace std;


Grid grid;

bool fillRandomData(){
    vector<DataPoint> records;
    for (int i = 0; i < NUMBER_OF_POINTS;i++){
        // records.push_back(DataPoint(i, 1,1));
        records.push_back(DataPoint(i, rand() % MAX_COORDINATE, rand() % MAX_COORDINATE));
    }
    ReWriteToFile("dataset.csv", records);
    return true;
}

bool processData(){
    vector<DataPoint> records = ReadFromFile("dataset.csv");
    for (int i = 0; i < records.size();i++){
        grid.insertDataPoint(records[i]);
    }
    return true;
}

bool printData(){
    cout << endl;
    cout << grid.y_splits.size() << ": size of y splits\n";
    for (int i = 0; i < grid.y_splits.size();i++){
        cout << grid.y_splits[i] << " ";
    }
    cout << endl;
    cout << grid.x_splits.size() << ": size of x splits\n";
    for (int i = 0; i < grid.x_splits.size(); i++)
    {
        cout << grid.x_splits[i] << " ";
    }
    cout << endl;
    cout << grid.cell_mapper.size() << ": size of cell mapper\n";
    for (auto i : grid.cell_mapper)
    {
        cout << i.first <<" "<< i.second->bucket->filename << endl;
    }
    return true;
}

int main(){
    int input;
    while(true){
        cout << "fill random data to file 1:\n\ninsert one by one from file to grid 2:\n\nprint datas 3:\n\nexit 0:\n\nyour input :";
        cin >> input;
        switch (input)
        {
        case 1:
            fillRandomData();
            break;
        case 2:
            processData();
            break;
        case 3:
            printData();
            break;
        case 0:
            return 0;
        default:
            continue;
        }
    }
}
#include<iostream>
#include<unordered_map>
#include<vector>
#include<string>
#include <fstream>
#include<sstream>
#include "constants.cpp"

using namespace std;

int cell_id = 0;


// data point 

class DataPoint{
    public:
    int id, x, y;
    DataPoint(int id, int x, int y) : id(id), x(x), y(y){}
    string asString(){
        return to_string(id) + "," + to_string(x) + "," + to_string(y);
    }
};

// data point end




// file

vector < DataPoint> ReadFromFile(string file_name){
    file_name = "./data/"+file_name;
    fstream fin;
    fin.open(file_name, ios::in);
    
    vector<string> row;
    vector<DataPoint> records;

    string line, word, temp;
    while (fin >> temp)
    {
        stringstream s(temp);
        row.clear();
        while (getline(s, word, ','))
            row.push_back(word);
        DataPoint dp(stoi(row[0]),stoi(row[1]),stoi(row[2]));
        records.push_back(dp);
    }
    fin.close();
    return records;
}

bool ReWriteToFile( string file_name,vector < DataPoint> records){
    file_name = "./data/"+file_name;
    fstream fout;
    fout.open(file_name, ios::out | ios::trunc);
    
    for (int i = 0; i < records.size();i++)
    {
        fout << records[i].asString()<< endl;
    }
    fout.close();
    return true;
}

bool RemoveFromFile( string file_name){
    file_name = "./data/"+file_name;
    fstream fout;
    fout.open(file_name, std::ofstream::out | std::ofstream::trunc);
    fout.close();
    return true;
}

bool WriteToFile( string file_name,DataPoint record){
    file_name = "./data/"+file_name;
    fstream fout;
    fout.open(file_name, std::ios_base::app	);
    fout << record.asString()<< endl;
    fout.close();
    return true;
}
// file end
int bucket_count =0;
// bucket
class Bucket{
    
    public:
    string filename;
    int cur_size;

    Bucket(){
        filename = to_string(++bucket_count)+".csv";
        cur_size = 0;
    }
    bool insert(DataPoint dp){
        WriteToFile(filename,dp);
        cur_size++;
        return true;
    }

    bool remove(){
        RemoveFromFile(filename);
        cur_size=0;
        return true;
    }

    vector<DataPoint> getAll(){
        return ReadFromFile(filename);
    }
};
//

// cell

class Cell{
public:
    Bucket *bucket;
    int x_begin, x_end;
    int y_begin, y_end;
    string filename;

    Cell(int id, Bucket *bucket,int x_begin, int x_end, int y_begin, int y_end): x_begin(x_begin),x_end(x_end),y_begin(y_begin),y_end(y_end), filename(to_string(id)+".csv"), bucket(bucket){ }
    vector<DataPoint> getAll(){
        vector<DataPoint> all = bucket->getAll(), cur;
        for (int i = 0; i < all.size(); i++)
        {
            if(all[i].x >x_begin && all[i].x<=x_end && all[i].y>y_begin && all[i].y<=y_end){
                cur.push_back(all[i]);
            }
        }
        return cur;
    }
    int curSize(){
        return getAll().size();
    }
};

// cell end

string getCellHash(int lower_x, int lower_y, int upper_x, int upper_y){
    return to_string(lower_x)+"-"+to_string(lower_y)+"-"+to_string(upper_x)+"-"+to_string(upper_y);
}

vector<int> tokenize(string s, string del = "-")
{
    int start = 0;
    int end = s.find(del);
    vector<int> nums;
    while (end != -1)
    {
       nums.push_back(stoi(s.substr(start, end - start)));
        start = end + del.size();
        end = s.find(del, start);
    }
    return nums;
}

vector<int> getHashPoints(string cellHash){
    return tokenize(cellHash, ",");
}

class Grid
{
    public:

    vector<int> x_splits, y_splits;
    unordered_map<string,Cell*> cell_mapper;

    Grid()
    {
        x_splits.push_back(-1);
        x_splits.push_back(MAX_COORDINATE);
        y_splits.push_back(-1);
        y_splits.push_back(MAX_COORDINATE);
        Bucket *b = new Bucket();
        Cell *initialCell = new Cell(++cell_id,b, -1, MAX_COORDINATE, -1, MAX_COORDINATE);
        cell_mapper[getCellHash(-1, -1, MAX_COORDINATE, MAX_COORDINATE)] = initialCell;
    }


    bool insertDataPoint(DataPoint dp){
        int x_begin=0, x_end=0, y_begin=0, y_end=0;
        //
        for (int i = 0; i < x_splits.size();i++){
            if(x_splits[i]>=dp.x){
                x_begin = x_splits[i - 1];
                x_end = x_splits[i];
                break;
            }
        }
        for (int i = 0; i < y_splits.size();i++){
            if(y_splits[i]>=dp.y){
                y_begin = y_splits[i - 1];
                y_end = y_splits[i];
                break;
            }
        }
        //
        string cellHash = getCellHash(x_begin,y_begin, x_end , y_end);
        // cout << cellHash << endl
        //      << dp.asString() << endl;
        if (cell_mapper.find(cellHash) != cell_mapper.end())
        {
            
            Cell *cur_cell = cell_mapper[cellHash];

            // cout << cellHash << "  =>"
            //      << dp.asString() << "=>" << cur_cell->curSize()<<" "<<cur_cell->bucket->filename<<" "<<cur_cell->bucket->cur_size << endl;

            if(cur_cell->bucket->cur_size>=BUCKET_SIZE){
                if(cur_cell->curSize()>=BUCKET_SIZE){
                    //split
                    split(cellHash);
                }
                else{
                    // allocate new for current
                    // new Cell(++cell_id,temp->bucket, less_cur_split_point,cur_split_point,y_splits[i-1],y_splits[i]);
                    Bucket *newBucket = new Bucket();
                    vector<DataPoint> oldContent, newContent, curCellContent;
                    oldContent = cur_cell->bucket->getAll();
                    curCellContent = cur_cell->getAll();
                    cur_cell->bucket->remove();
                    int old_bucket_size=oldContent.size(), new_bucket_size;
                    unordered_map<int, bool> curIds;
                    for (int i = 0; i < curCellContent.size();i++){
                        curIds[curCellContent[i].id] = true;
                    }
                    for (int i = 0; i < oldContent.size();i++){
                        if(curIds[oldContent[i].id]){
                            newBucket->insert(oldContent[i]);
                        }
                        else{
                            cur_cell->bucket->insert(oldContent[i]);
                        }
                    }
                    new_bucket_size = (cur_cell->bucket->getAll()).size();
                    // cout << old_bucket_size << " ::: " << new_bucket_size;
                    cur_cell->bucket = newBucket;
                }
                //restart insertion
                return insertDataPoint(dp);
            }
            else{
                cur_cell->bucket->insert(dp);
            }
        }
        //
        return true;
    }

    bool isXsplitAxis = true;

    bool split(string cellHash){
        Cell *cur_cell = cell_mapper[cellHash];
        bool currentIsXsplitAxis = isXsplitAxis;
        int cur_split_point;
        // getting the split point
        vector<DataPoint> cur_cell_points = cur_cell->getAll();
        vector<int> points;
        for(auto i:cur_cell_points){
            if(currentIsXsplitAxis){
            points.push_back(i.x);
            // cout << i.x << " ";
            }
            else{
            points.push_back(i.y);
            // cout << i.y << " ";
            }
        }
        
        isXsplitAxis = !isXsplitAxis;
        sort(points.begin(), points.end());
        int total_size = points.size();
        cur_split_point = (points[((total_size+1) / 2)-1] +points[((total_size) / 2)-1]+1)/2 ;
        // cout <<"("<<cur_split_point<<")"<< endl;
        int less_cur_split_point, more_cur_split_point;
        //
        // if(cellHash=="73,-1,216,78"){
        // }
        if(currentIsXsplitAxis){
            for (int i = 0; i < x_splits.size();i++){
                if(x_splits[i]>=cur_split_point){
                    less_cur_split_point = x_splits[i - 1];
                    more_cur_split_point = x_splits[i];
                    break;
                }
            }
            x_splits.push_back(cur_split_point);
            sort(x_splits.begin(), x_splits.end());

            for (int i = 1; i < y_splits.size();i++){
                Cell *temp = cell_mapper[getCellHash(less_cur_split_point, y_splits[i - 1], more_cur_split_point, y_splits[i])];
                cell_mapper.erase(getCellHash(less_cur_split_point, y_splits[i - 1], more_cur_split_point, y_splits[i]));
                cell_mapper[getCellHash(less_cur_split_point,y_splits[i-1],cur_split_point,y_splits[i])]=new Cell(++cell_id,temp->bucket, less_cur_split_point,cur_split_point,y_splits[i-1],y_splits[i]);
                cell_mapper[getCellHash(cur_split_point,y_splits[i-1],more_cur_split_point,y_splits[i])]=new Cell(++cell_id,temp->bucket, cur_split_point,more_cur_split_point,y_splits[i-1],y_splits[i]);
                // cell_mapper[getCellHash(less_cur_split_point,y_splits[i-1],cur_split_point,y_splits[i])]=new Cell(++cell_id,temp->bucket);
                // cell_mapper[getCellHash(less_cur_split_point,y_splits[i-1],cur_split_point,y_splits[i])]=new Cell(++cell_id,temp->bucket);
            }
        }
        else{
            for (int i = 0; i < y_splits.size();i++){
                if(y_splits[i]>=cur_split_point){
                    less_cur_split_point = y_splits[i - 1];
                    more_cur_split_point = y_splits[i];
                    break;
                }
            }
            y_splits.push_back(cur_split_point);
            sort(y_splits.begin(), y_splits.end());

            for (int i = 1; i < x_splits.size();i++){
                Cell *temp = cell_mapper[getCellHash(x_splits[i - 1],less_cur_split_point,x_splits[i] , more_cur_split_point )];
                cell_mapper.erase(getCellHash(x_splits[i - 1],less_cur_split_point,x_splits[i] , more_cur_split_point ));
                cell_mapper[getCellHash(x_splits[i - 1],less_cur_split_point,x_splits[i] , cur_split_point )]=new Cell(++cell_id,temp->bucket, x_splits[i - 1],x_splits[i],less_cur_split_point,cur_split_point);
                cell_mapper[getCellHash(x_splits[i - 1],cur_split_point,x_splits[i] , more_cur_split_point )]=new Cell(++cell_id,temp->bucket, x_splits[i - 1],x_splits[i],cur_split_point,more_cur_split_point);
                // cell_mapper[getCellHash(less_cur_split_point,y_splits[i-1],cur_split_point,y_splits[i])]=new Cell(++cell_id,temp->bucket);
                // cell_mapper[getCellHash(less_cur_split_point,y_splits[i-1],cur_split_point,y_splits[i])]=new Cell(++cell_id,temp->bucket);
            }
        }
        //cout << less_cur_split_point << "|" <<cur_split_point<<"|"<< more_cur_split_point <<" & "<<currentIsXsplitAxis<<"|"<<cellHash<< endl;
        return true;
    }
};




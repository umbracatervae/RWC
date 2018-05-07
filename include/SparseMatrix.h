//
// Created by Paul on 4/19/2018.
//

#ifndef RW_SPARSEMATRIX_H
#define RW_SPARSEMATRIX_H

#include <iostream>

//Sparse matrix holds a 2D matrix of ints
//avoids storing 0 by using an offset and size variable to point to only the range of nonzero values in a matrix row
class SparseMatrix{
private:
    int rows, columns;
    int *offset;
    int *size;
    int **data;

public:
    SparseMatrix(int rows, int columns){
        if (rows>0 && columns>0){
            this->rows = rows;
            this->columns = columns;
            offset = new int[rows];
            size = new int[rows];
            data = new int* [rows];
            for (int r=0; r<rows; ++r){
                size[r] = 0;
                offset[r] = 0;
                data[r] = nullptr;
            }
        }
        else{
            this->rows = 0;
            this->columns = 0;
            offset = nullptr;
            size = nullptr;
            data = nullptr;
        }
    }
    ~SparseMatrix(){
        for(int r=0; r<rows; ++r){
            delete[] data[r];
        }
        delete[] offset;
        delete[] size;
        delete[] data;
        rows=columns=0;
    }
    void set(int row, int column, int value){
        if(row<0 || row>=rows || column < 0 || column >=columns) return; //out of bounds
        if(size[row] ==0){ //no data on this row yet
            if(value == 0) return; //no need to store a 0
            data[row] = new int[1];
            data[row][0] = value;
            size[row] = 1;
            offset[row] = column;
            return;
        }
        if(column>=offset[row]+size[row]){ //new value comes after existing data in row
            int newSize = column-offset[row]+1;
            int* newRow = new int[newSize];
            for(int i=0; i<size[row];++i){
                newRow[i] = data[row][i];
            }
            for(int i=size[row]; i<newSize; ++i){
                newRow[i] = 0;
            }
            newRow[column-offset[row]] = value;
            delete[] data[row];
            data[row] = newRow;
            size[row] = newSize;
            return;
        }
        std::cout << "UNSUPPORTED SPARSE MATRIX OPERATION" << std::endl;
        //for current RW, only supports sequential insertion
    }
    int get(int row, int column){
        if(row<0 || row >=rows) return 0;
        int index = column - offset[row];
        if(index<0 || index>=size[row]) return 0;
        return data[row][index];
    }

};


#endif //RW_SPARSEMATRIX_H

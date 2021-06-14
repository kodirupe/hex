#include <iostream>
#include <vector>
#include <cstdlib>
#include <stdlib.h>
#include <queue>
#include <utility>
#include <time.h>
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>
using namespace std;


//Defines the values for spaces taken during the game
//Blank is untaken,red is the players space, blue is computer space
enum player {blank,red,blue};

//Outputs for the spaces for the board
ostream& operator<<(ostream& os,  player p){
    switch(p){
        case blank: os << "."; break;
        case red: os << "R"; break;
        case blue: os <<"B";break;
    }
    return os;
}


class hex_game{

    public:
        
        //Constructor, takes a board size as an argument, makes a board that is board_size x board_size
        hex_game(int board_size): board_size(board_size){
            for(int i = 0;i < board_size;i++){
                board.push_back(vector<player>(board_size,blank)); // Make all spaces untaken
            }
        }


        //Returns true if player wins, false if computer wins
        bool play_game(){
            
            //Seed random number generator
            srand(time(0));

            int x,y; //Player move coodinates
            cout << "\nWelcome to hex! let the best player win" << endl;
            cout << "Player is 'R',computer is 'B', '.' is untaken" << endl;
            cout << "Coordinates go from 0 to " << board_size-1 << endl;
            cout << "Player must connect top and bottom, computer must connect left to right" << endl;
            while(true){
                print_board();
                
                //Get player move
                cout << "Make your move, enter the x coordinate: ";
                cin >> x;
                cout << "\nMake your move, enter the y coordinate: ";
                cin >> y;
                cout << "\n";

                //Invalid move, get valid input
                while(!valid_move(x,y)){
                    cout << "Invalid move\nEnter the x coordinate for your move: ";
                    cin >> x;
                    cout << "\nEnter the y coordinate for your move: ";
                    cin >> y;
                    cout << "\n";
                }

                //Modifies board for player move
                modify_board(x,y,red);

                //If player wins, game is over
                if(game_over(this->board,red))
                    return true;
                
                //Computer turn
                computer_move();

                //If computer wins, game is over
                if(game_over(this->board,blue))
                    return false;        
            }
        }

        
        friend ostream& operator<<(ostream& os,player p);
  
    private:

        
        vector<vector<player> > board;//Vector to make the board
        
        int board_size; //board will be board_size x board_size
        
        
        //Returns whether two vertices are connected
        bool is_connected(vector<vector<player> > h_board,int x1,int y1,int x2, int y2){

            //Bad coordinates, therefore not connected
            if(x1 < 0 || x1 >= h_board.size() || x2 < 0 || x2 >= h_board.size() ||
               y1 < 0 || y1 >= h_board.size()|| y2 < 0 || y2 >= h_board.size())
              return false;
            
            //Each internal space has 6 neighbors, dont have to worry about edge cases, already taken care of with
            //Checking that the coordinates are valid input
            if(x2 == x1-1 && y2 == y1){
                return true;
            }
            if(x2 == x1-1 && y2 == y1+1){
                return true;
            }
            if(x2 == x1 && y2 == y1-1){
                return true;
            }
            if(x2 == x1 && y2 == y1+1){
                return true;
            }
            if(x2 == x1+1 && y2 == y1-1){
                return true;
            }
            if(x2 == x1+1 && y2 == y1){
                return true;
            }
            return false;
        }

        //returns whether the space is red blue or untaken
        player space_type(vector<vector<player> > h_board,int x, int y){
            return h_board[x][y];
        }


        //Modifies board 
        void modify_board(int x, int y,player p){
            board[x][y] = p;
        }


        pair<int,int> monte_carlo(){
            vector<vector<player> > board_cpy = vector<vector<player> >(board);
            int x_final,y_final,most_wins = -1;

            //Check each space for best move
            for(int i = 0;i < board_cpy.size();i++){
                for(int j = 0;j < board_cpy.size();j++){

                    
                    //If the move is valid, run a trial
                    if(board_cpy[i][j] == blank){
                        board_cpy[i][j] = blue;
                        vector<pair<int,int> > valid_moves;

                        //Make a vector of valid moves to be shuffled
                        for(int u = 0;u < board_cpy.size();u++){
                            for(int t = 0;t < board_cpy.size();t++){
                                if(board_cpy[u][t] == blank)
                                    valid_moves.push_back(pair<int,int>(u,t));
                            }
                        }
                        int num_wins = 0;
                        //Run trials
                        for(int k = 0;k < 1000;k++){
                            //obtain time based seed
                            unsigned seed = chrono::system_clock::now().time_since_epoch().count();
                            shuffle(valid_moves.begin(),valid_moves.end(),default_random_engine(seed));
                            player curr_p_type = red;

                            for(pair<int,int> space: valid_moves){
                                board_cpy[space.first][space.second] = curr_p_type;
                                if(curr_p_type == red){
                                    curr_p_type = blue;
                                }else{
                                    curr_p_type = red;
                                } 
                            }
                            if(game_over(board_cpy,blue))
                                num_wins++;
                            
                        }

                        //New best move
                        if(num_wins > most_wins){
                            most_wins = num_wins;
                            x_final = i;
                            y_final = j;
                        }
                        
                        //Remake board copy to be the same as the board
                        for(pair<int,int> space: valid_moves){
                            board_cpy[space.first][space.second] = blank;
                        }
                        board_cpy[i][j] = blank;
                    }
                }
            }
            return pair<int,int>(x_final,y_final);
        }
        //Just picks a random valid space
        void computer_move(){
           
            pair<int,int> move = monte_carlo();

            modify_board(move.first,move.second,blue);
        }

        //Returns a number between 0 and max
        int random_number(int max){
            return rand()%max;
        }

        //Returns true if the move is valid, false if the move is invalid
        bool valid_move(int x,int y){
            
            //Invalid coordinate
            if(x < 0 || x >= board_size || y < 0 || y >= board_size)
                return false;
            
            player coordinate = board[x][y];
            
            //Check that space isn't already taken
            if(coordinate == blank){
                return true;
            }else{ //Invalid Move
                return false;
            }
        }


        //Returns true if the inputted player has won
        //Uses a depth first search type algorithm
        bool game_over(vector<vector<player> > h_board,player p_type){
            queue<pair<int,int> > unvisited; //Keeps track of unvisited coordinates
            vector<pair<int,int> > visited; //Keeps track of visited coordinates
            
            //Check if any space on the first row is taken by the player
            if(p_type == red){ 
                for(int y = 0;y < h_board.size();y++){
                    if(space_type(h_board,0,y) == red){
                        unvisited.push(pair<int,int>(0,y));
                    }
                }
            }


            //Check if any space in the first column is taken by the computer
            if(p_type == blue){
                for(int x = 0;x < h_board.size();x++){
                    if(space_type(h_board,x,0) == blue){
                        unvisited.push(pair<int,int>(x,0));
                    }
                }
            }


            while(!unvisited.empty()){
                pair<int,int> space = unvisited.front();//Grab first univisted coordinate
                unvisited.pop();

                //Initalize coordinates for easy use
                int x = space.first;
                int y = space.second;

                //If its the player and they have reached the last row, they win
                if(p_type == red && x == h_board.size()-1){
                   return true;
                }

                //If its the computer and they have reached the last column, they win
                if(p_type == blue && y == h_board.size()-1){
                    return true;
                }

                //Space has been visited
                visited.push_back(space);


                //Check if each neighbor is is taken by the inputted player and that it hasnt been visited yet, if it hasnt been visited
                //Add it to unvisited queue

                if(is_connected(h_board,x,y,x-1,y) && space_type(h_board,x-1,y) == p_type &&
                   !contains_pair(visited,x-1,y)){
                     unvisited.push(pair<int,int>(x-1,y));
                }

                if(is_connected(h_board,x,y,x-1,y+1) && space_type(h_board,x-1,y+1) == p_type &&
                   !contains_pair(visited,x-1,y+1)){
                     unvisited.push(pair<int,int>(x-1,y+1));
                }

                if(is_connected(h_board,x,y,x,y-1) && space_type(h_board,x,y-1) == p_type &&
                   !contains_pair(visited,x,y-1)){
                     unvisited.push(pair<int,int>(x,y-1));
                }

                if(is_connected(h_board,x,y,x,y+1) && space_type(h_board,x,y+1) == p_type &&
                   !contains_pair(visited,x,y+1)){
                    unvisited.push(pair<int,int>(x,y+1));
                }

                if(is_connected(h_board,x,y,x+1,y-1) && space_type(h_board,x+1,y-1) == p_type &&
                   !contains_pair(visited,x+1,y-1)){
                     unvisited.push(pair<int,int>(x+1,y-1));
                }

                if(is_connected(h_board,x,y,x+1,y) && space_type(h_board,x+1,y) == p_type &&
                   !contains_pair(visited,x+1,y)){
                    unvisited.push(pair<int,int>(x+1,y));
                }
             }
             return false; //They have not won, game isn't over
          }
        
        //Helper function that repeats and returns a given string s, n times
        string repeat(string s,int n){
            string product = "";
            for(int i = 0;i < n;i++){
                product += s;
            }            
            return product;
        }

        //Prints the board
        void print_board(){
            int spaces = 0; //Counter for the amount of spaces for each row to make the board a parallelogram instad of square
            for(int i = 0;i < board_size;i++){
                cout << repeat(" ",spaces++);
                for(int j = 0;j < board_size;j++){
                    if(j == board_size-1){
                        cout << space_type(this->board,i,j) << endl;
                    }else{
                        cout << space_type(this->board,i,j) << " - ";
                    }
                }
                cout << repeat(" ",spaces++);
                if(i < board_size-1){
                    cout << repeat("\\ / ",board_size-1) << "\\" << endl;
                }
            }
            cout << endl;
        }

        //returns whether or not a vector contains an x,y pair
        bool contains_pair(vector<pair<int,int > > v,int x,int y){
            for(pair<int,int> p: v){
                if(p.first == x && p.second == y){
                    return true;
                }
            }
            return false;
        }

};

int main(){
    hex_game h(5);
    if(h.play_game()){
        cout << "You win!!" << endl;
    }else{
        cout << "The computer wins" << endl;
    }

    return 0;
}
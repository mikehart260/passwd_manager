/*
Author: Michael Hart
Date: 05/28/2025
Description: Command line password manager with XOR encryption. It uses a polynomial rolling hash function for quick lookup. 
Instructions:
Click run button to use G++
*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <memory>

// Use string hashing to populate a vector, then when the user wants to look up their password for a specific website, I can use the hashing function
// to locate it. 
class Entry{
    private:
        // TODO: Handle encryption key better. ie ask user for a master password. 
         int key = 35;
    public:
        std::string website;
        std::string raw_p;
        std::string encrypted_p;
        Entry(std::string w) : website(w), raw_p(""), encrypted_p("") {}

        // Create Entry object. By default (for adding new passwords), the password gets encrypted.
        Entry(std::string w, std::string p, std::string encrypt_decrypt = "encrypt") : website(w) {
            std::string temp = p;
            for ( int i = 0; i < temp.size(); i++ ){
                temp[i] ^= key;
            }
            if ( encrypt_decrypt[0] == 'e' ){
                raw_p = p;
                encrypted_p = temp;
            } else if ( encrypt_decrypt[0]=='d' ){
                encrypted_p = p;
                raw_p = temp;
            } else {
                std::cerr << "Invalid encrypt/decrypt argument.\n";
            }
        }
};

class Hashtable {
    public:
        int m = 97;
        int p = 31; // prime number roughly equal to number of letters in alphabet
        std::vector < std::vector<std::shared_ptr<Entry>> > arr = std::vector<std::vector<std::shared_ptr<Entry>>>(97);
        
        Hashtable(){};
        int hash( std::string s ){
            // polynomial rolling hash function
            unsigned long long sum = 0;
            for ( int i = 0; i < s.size(); i++ ){
                int ascii = int(s[i]);
                sum = sum + ascii * pow(p, i);
            }
            return sum % m; 
        }

        // insert entry into hashmap using the hash function. Doesn't insert the website if the website exists.
        // TODO: Handle duplicate values by asking to overwrite password.
        bool insert( std::shared_ptr<Entry> e ){
            // call hash function to find index
            std::cout << "Inserting encrypted login...\n";
            int index;
            if ( !search(e, index) ){
                arr[index].push_back(e);
                std::cout << "Insert completed.\n";
                return true;
            } else {
                std::cout << "Duplicate website.\n";
                return false;
            }
        }
        // returns true if entry found in hashmap.
        bool search( std::shared_ptr<Entry> e, int &index ){
            index = hash(e->website);
            for (int i = 0; i < arr[index].size(); i++){
                if ( arr[index][i]->website == e->website ){
                    return true;
                }
            }
            return false;
        }
        // Search the hashmap for a website. 
        bool search( std::string website, int &index ){
            // make a fake entry, then call search.
            std::shared_ptr<Entry> e( new Entry(website,""));
            return search(e, index);
        }

        // Returns a pointer to Entry object for a website. If not found, returns NULL. 
        std::shared_ptr<Entry> get_entry(std::string website){
            int index;
            if ( !search(website, index) ) {
                std::cout << "Website not found...\n";
                return NULL;
            }
            for ( int i = 0; i < arr[index].size(); i++ ){
                if ( arr[index][i]->website == website ){
                    return arr[index][i];
                }
            }
            return NULL;
        }

        // load hashtable with entries from disk.
        void load(){
            std::cout << "Loading...\n";
            std::fstream file1("passwords.txt", std::ios::in);
            std::string w, p;
            if (file1.is_open()){
                while ( getline(file1, w, ',') ) {
                    getline(file1, p);
                    // create an entry from the passwords file and decrypt the password to get the original password. 
                    std::shared_ptr<Entry> e1(new Entry(w, p, "decrypt"));
                    insert(e1);
                }
                file1.close();
                std::cout << "Load completed!\n";
            } else {
                std::cerr << "File failed to open!\n";
            }
        }
        // creates a new encrypted entry from user input then calls insert function.
        void add_new_entry(){
            // website and password variables.
            std::string website, password;
            // Get website and password from the user.
            std::cout << "Enter Website: ";
            std::getline(std::cin, website);

            std:: cout << "Enter Password: ";
            std::getline(std::cin, password);
            
            std::shared_ptr<Entry> e(new Entry(website, password));
            // add the new entry to hashtable.
            if ( insert(e) ){
                std::cout << "Added.\n";
            } else {
                std::cout << "Failed to add.\n";
            }
        }

        // remove a website from the hashtable. 
        bool remove(){
            std::cout << "website: ";
            std::string website;
            std::getline(std::cin, website);
            int index;
            // if website isn't found, return false. 
            if (!search(website, index)){
                return false;
            }    
            for (std::vector<std::shared_ptr<Entry>>::iterator it = (arr[index]).begin(); it != arr[index].end();){
                if ( (*it)->website == website ){
                    arr[index].erase(it);
                    std::cout << "removed.";
                    return true;
                } 
                it++;
            }
            return false;
        }

        // write entries to file
        bool write_to_file(){
            std::fstream file("passwords.txt", std::ios::out);
            if ( !file.is_open() ){
                std::cout << "File failed to open.\n";
                return false;
            }
            for ( int i = 0; i < arr.size(); i++ ){
                for ( int j = 0; j < arr[i].size(); j++){
                    file << arr[i][j]->website << "," << arr[i][j]->encrypted_p << "\n";
                }
            }
            return true;
        }
        // list all websites
        void list_all() {
            for ( int i = 0; i < arr.size(); i++ ){
                for ( int j = 0; j < arr[i].size(); j++ ){
                    std::cout << arr[i][j]->website << "\n";
                }
            }
        }
};

int main() {
    std::shared_ptr<Hashtable> table(new Hashtable());
    // on start, load hashmap from passwords.txt 
    table->load();
    // get command from user
    // TODO: Make this keep asking for input after a wrong command is issued or until a quit button is entered.
    
    while ( true ){
        std::cout << "Enter command. (type help for list of commands, quit to quit.): ";
        std::string command;
        std::getline(std::cin, command);

        if ( command == "help" ){
            std::cout << "add, delete, get, list, help, quit\n";
        } else if ( command == "add" ){
            // call add function
            table->add_new_entry();
        } else if ( command == "delete" ) {
            table->remove();
        } else if ( command == "get" ) {
            // give the unencrypted password to the user.
            std::string website;
            std::cout << "website: ";
            std::getline(std::cin, website);
            std::shared_ptr<Entry> e = table->get_entry(website);
            if ( e != NULL ){
                std::cout << e->raw_p << "\n";
            } else {
                std::cout << "website doesn't exist.\n";
            }
        } else if ( command == "list" ) {
            table->list_all();
        } else if ( command == "quit" ) {
            table->write_to_file();
            break;
        } else if ( command == "" ) {

        } else {
            std::cout << "Command not found!\n";
        }
    }
        
    // take data from hashtable and write it to the passwords file.
    return 0;
}
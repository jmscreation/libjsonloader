/*
    Author:     jmscreator
    License:   Always free to use forever (See MIT License)

    This is an example program which demonstrates how to use the rapidjson wrapper

*/

#include "jsonloader.h" // rapidjson parsing library wrapper

#include <fstream> // load from the file
#include <iostream> // cout + string
#include <vector>
#include <sstream>


// Simple struct to load the json data into
struct Person {
    std::string name;
    float age;

    struct Hobby { // hobby struct
        std::string name;
        uint32_t rating;
    };

    std::vector<Hobby> hobbies;
    std::vector<std::string> inventory;
};

std::vector<Person> people; // list of people loaded into memory


static JsonLoader json;

bool SavePerson(const std::string& path, const Person& person) {
    std::ofstream file(path, std::ios::binary | std::ios::out); // output file

    if(!file.is_open()){
        std::cout << "failed to save \"" << path << "\"\n";
        return false;
    }
    
    json.clearData();

    json.saveProperty("name", person.name);
    json.saveProperty("age", person.age);

    JsonLoader::Object hobbies;
    for(const Person::Hobby& hobby : person.hobbies){
        json.saveProperty(hobbies, hobby.name.data(), hobby.rating);
    }
    json.saveProperty("hobbies", hobbies);

    JsonLoader::Array inventory;
    for(const std::string& str : person.inventory){
        if(!json.appendArrayValue(inventory, str)) {
            std::cout << "warning: inventory element failed to export\n";
        }
    }
    json.savePropertyArray("inventory", inventory);

    std::string rawData; // where to store raw json data

    if(!json.exportData(rawData)){
        std::cout << "failed to export person data\n";
        return false;
    }

    file << rawData;

    file.close();

    return true;
}

bool LoadPerson(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::in); // input file

    if(!file.is_open()){
        std::cout << "failed to load \"" << path << "\"\n";
        return false;
    }

    std::string rawData; // where to store raw json data

    std::stringstream buf;
    buf << file.rdbuf(); // load data from file
    rawData = buf.str();

    file.close();
    
    JsonLoader::Object p;

    if(!json.parseStringObject(rawData, p)){ // parse the raw json data
        std::cout << "failed to parse json data\n";
        return false;
    }

    Person person;

    if(!json.loadProperty(p, "name", person.name)){
        std::cout << "failed to load person's name\n";
        return false;
    }
    if(!json.loadProperty(p, "age", person.age)){
        std::cout << "failed to load person's age\n";
        return false;
    }

    JsonLoader::Object hobbies;
    if(json.loadProperty(p, "hobbies", hobbies)){
        for(auto it = hobbies.MemberBegin(); it != hobbies.MemberEnd(); ++it){ // iterate over hobbies
            Person::Hobby hobby;
            if(json.loadValue(it->name, hobby.name)
            && json.loadValue(it->value, hobby.rating)){
                person.hobbies.emplace_back(hobby); // add hobby to person's hobby list
            }
        }
    } else {
        std::cout << "failed to load person's hobbies\n";
    }

    JsonLoader::Array inventory;
    if(json.loadPropertyArray(p, "inventory", inventory)){
        for(JsonLoader::Object* it = inventory.Begin(); it < inventory.End(); ++it){ // iterate over inventory array
            std::string item; // where to store the string
            if(json.loadValue(*it, item)){ // load string from array element
                person.inventory.push_back(item); // add inventory item to person's inventory list
            }
        }
    } else {
        std::cout << "failed to load person's inventory\n";
    }

    people.emplace_back(person); // push the person onto the list of people

    return true;
}

int main() {

    {
        JsonLoader test;
        test.parseString("{}");

        JsonLoader::Object obj;
        for(int i=0; i < 4; ++i){
            JsonLoader::Object sub;

            test.saveProperty(sub, "index", i);
            test.saveProperty(sub, "name", std::to_string(i));
            
            if(i==0) test.saveProperty(obj, "sub_obj_0", sub);
            if(i==1) test.saveProperty(obj, "sub_obj_1", sub);
            if(i==2) test.saveProperty(obj, "sub_obj_2", sub);
            if(i==3) test.saveProperty(obj, "sub_obj_3", sub);

            //test.saveProperty(obj, (std::string("sub_obj_") + std::to_string(i)).c_str() , sub);
        }
        

        std::cout << test.toString(obj) << "\n";

        test.saveProperty("test", obj);

        std::cout << test.toString() << "\n";
        return 1;
    }



    json.displayErrors = true;

    json.parseString("{}");

    // load people files
    LoadPerson("person1.json");
    LoadPerson("person2.json");

    // print the information for each person
    for(Person& person : people){
        std::cout << person.name << ":\n"
                << "\tage: " << person.age << "\n"
                << "\thobbies:" << "\n";

        for(Person::Hobby& hobby : person.hobbies){
            std::cout << "\t\t" << hobby.name << " -> " << hobby.rating << " / 10\n";
        }
        
        std::cout << "\tinventory:\n";
        for(std::string& item : person.inventory){
            std::cout << "\t\t" << item << "\n";
        }
    }

    // save one of the people to personOutput file

    SavePerson("outputPerson.json", people[rand() % people.size()]);
    
    return 0;
}
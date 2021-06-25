#include <stdio.h>
#include <json-c/json.h>

int main()
{
    json_object *obj = json_object_new_object();
    json_object *obj2 = json_object_new_object();

    json_object_object_add(obj, "int", json_object_new_int(123));
    json_object_object_add(obj, "int", json_object_new_int(123));
    json_object_object_add(obj2, "int2", json_object_new_int(123));
    json_object_object_add(obj2, "int2", json_object_new_int(123));
    json_object_object_add(obj, "obj", obj2);

                        
    


    
    printf("\tResult:%s\r\n", json_object_to_json_string(obj));

	json_object_to_file("json.out", obj);
}

                            Layer 6 v0.7
                            ============

                         Copyright 2001 - 2012
                        kunalkandekar@gmail.com

Layer6 is a binary data representation format, serialization protocol and coll-
ection of APIs for encoding structured data using dynamic, strongly typed, self-
describing schemas. The main distinguishing features are:
1. the self-describing message metadata, or "schema", is defined dynamically as
   fields are added or removed programmatically (and hence need not be 
   predefined by the user); 
2. the self-describing metadata is maintained in a message header segment that 
   is logically separated from and precedes the data segment. This enables the
   potential for schema re-use for messages with repetitive formats. 

Languages currently supported: C, C++, Java, Python

Tested on MacOS X (10.6.8) and Linux (Ubuntu 10.10).

Code Samples
============
C (C99)
-------

    /* Serialization */
    l6msg ms;
    l6msg_init(&ms);                                //initialize
    l6msg_add_int64(&ms, (long long)42);            //add 64-bit int @ index 0
    l6msg_set_float_named(&ms, "myfloat", 23.56);   //named float @ 1
    l6msg_set_string(&ms, 99, "Lorem ipsum");       //copy of string with ID @ 2
    int a[] = { 1, 2, 3, 5, 9, 11, 13 };
    l6msg_add_int_array_ptr(&ms, a, sizeof(a)/sizeof(int)); //ref to int array @ 3
    l6msg_add_int_array(&ms, a, sizeof(a)/sizeof(int));    //copy of int array @ 4
    a[4] = 7;                                       //field changed @ 3, but not @ 4

    int left = 0;                                   //bytes unserialized
    char buffer[4096];
    int len = l6msg_serialize(&ms, buffer, sizeof(buffer), &left);
    if(len < 0) {
        //error
    }
    printf("Serialized %d bytes, %d still left\n", len, left);
    l6msg_free(&ms);
   
   
    /* Deserialization */
    l6msg mr;
    l6msg_init(&mr);                                //initialize
    int left = 0;                                   //bytes un-deserialized
    int len = l6msg_deserialize(&mr, buffer, sizeof(buffer), &left);
    if(len < 0) {
        //error
    }
    printf("Deserialized %d bytes, %d still left\n", len, left);

    long long l = 0;
    float f = 0;
    char str[32];
    l6msg_get_int64_at_index(&mr, 0, &l);           //get 64-bit int @ index 0
    l6msg_get_float_named(&mr, "myfloat", &f);      //get float by name 
    l6msg_get_string(&mr, 99, &str, sizeof(str));   //get string copy by ID
    printf("Long %ld, float %f and string '%s'\n", l, f, str);        
    int *a, c;
    l6msg_get_int_array_ptr_at_index(&mr, 4, &a, &c); //get ref to int array @ 4
    printf("Array of %d ints\n", c);                //"Array of 7 ints"
    l6msg_free(&mr);                                //can free now

 
C++
---
    /* Serialization */
    Layer6Msg ms;                                   //RAII
    ms.setExceptionsEnabled(false);                 //disable exceptions
    ms.addInt64((long long)42);                     //add 64-bit int @ index 0
    ms.setFloat("myfloat", 23.56);                  //named float @ 1
    ms.setString(8, "Lorem ipsum");                 //copy of string with ID @ 2
    int a[] = { 1, 2, 3, 5, 9, 11, 13 };
    ms.addIntArrayPtr(a, sizeof(a)/sizeof(int));    //ref to int array @ 3
    ms.addIntArray(a, sizeof(a)/sizeof(int));       //copy of int array @ 4
    a[4] = 7;                                       //field changed @ 3, but not @ 4

    int left = 0;                                   //bytes unserialized
    int len = ms.serialize(buffer, sizeof(buffer), &left);
    if(len < 0) {
        //error
    }
    std::cout<<"Serialized "<<len<<" bytes, "<<left<<" still left\n";


    /* Deserialization */
    try {                                           //Exceptions on by default
        Layer6Msg *mr = new Layer6Msg();
        int left = 0;                               //bytes undeserialized
        int len = mr->deserialize(buffer, sizeof(buffer), &left);
        std::cout<<"Deserialized "<<len<<" bytes, "<<left<<" still left\n";

        long long l = 0;
        float f = 0;
        l = mr->getInt64At(0);                      //get 64-bit int at index 0
        f = mr->getFloat("myfloat");                //get float by name
        char str[32];
        mr->getString(99, &str, sizeof(str));       //get string copy by ID    
        std::cout<<"Long "<<l<<", float "<<f<<" and string '"<<str<<"'\n";
        int c;
        int *a = mr.getIntArrayPtrAt(4, &c);        //get ref to int array @ 4
        std::cout<<"Array of "<<c<<" ints\n";       //"Array of 7 ints"
        delete mr;
    }
    catch(int &ex) {
        std::cout<<"Exception "<<Layer6Msg::getErrorStrForCode(ex)<<"\n";
    }
    
Java
----
    try {
        /* Serialization */
        Layer6Msg ms = new Layer6Msg();
        ms.addLong(42);                             //add 64-bit int @ index 1
        ms.setFloat("myfloat", (float)23.56);       //named float @ 2
        ms.setStringRef(99, "Lorem ipsum");         //string copy with ID @ 3
        int [] a = { 1, 2, 3, 5, 9, 11, 13 };
        ms.addIntArrayRf(a, a.length);              //ref to int array @ 3
        ms.addIntArray(a, a.length);                //copy of int array @ 4
        a[4] = 7;                                   //field changed @ 3, but not @ 4
        int left = 0;                               //bytes unserialized

        byte [] buffer = new byte[ms.size()];       //buffer of the right size
        int len = ms.serialize(buffer, 0, buffer.length);
        System.out.println("Serialized "+len+" bytes, "+left+" still left\n");
    
        /* Deserialization */
        Layer6Msg mr = new Layer6Msg();
        int left = 0;                               //bytes undeserialized
        int len = mr.deserialize(buffer, 0, buffer.length);
        System.out.println("Deserialized "+len+" bytes, "+left+" still left\n");

        long   l = mr.getInt64At(0);                //get 64-bit int at index 0
        float  f = mr.getFloat("myfloat");          //get float by name
        String s = mr.getString(99);                //get string copy by ID    
        System.out.println("Long "+l+", float "+f+" and string '"+str+"'\n");
        int [] a = mr.getIntArrayPtrAt(4);          //get ref to int array @ 4
        System.out.println("Array of "+a.length+" ints\n");   //"Array of 7 ints"
    }
    catch(Layer6Exception l6Ex) {
        l6Ex.printStackTrace();
    }


Python
------
    try:
        # Serialization
        ms = Layer6Msg()
        ms.add_long(42)                         # add 64-bit int @ index 0
        ms.set_float_named("myfloat", 23.56)    # named float @ 1
        ms.set_string(99, "Lorem ipsum")        # copy of string with ID @ 2
        a = [ 1, 2, 3, 5, 9, 11, 13 ]
        ms.add_int_array_ptr(a)                 # ref to int array @ 3
        ms.add_int_array(a)                     # copy of int array @ 4
        a[4] = 7                                # field changed @ 3, but not @ 4
        buf = array.array('c', 0 * ms.size())
        length, left = ms.serialize()
        print "Serialized ", len(buf), " bytes,", left, " bytes left."
        
        # Deserialization
        mr = Layer6Msg()
        ret, left = mr.deserialize(buf, len(buf))
        print "Deserialized ", ret, " bytes,", left, " bytes left."
        l = mr.get_int64_at(0)                  # get 64-bit int at index 0
        f = mr.get_float_named("myfloat")       # get float by name
        s = mr.get_string(99)                   # get string copy by ID    
        print "Got long ", l, ", float ", f, " and string '", str
        a = mr.get_int_array_ptr_at(4)          # get ref to int array @ 4
        print "Got array of", len(a),"ints"     # "Got array of 7 ints"

    except Layer6Error as l6e:
        print "Unexpected L6 error:", l6e

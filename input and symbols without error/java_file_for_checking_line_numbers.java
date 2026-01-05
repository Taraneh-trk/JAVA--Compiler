class Main {
    public static void main(String [] args) {
        int x;
        int x;  // Error 1: Duplicate variable in same scope
        
        int result;
        result = y + 1;  // Error 5: Variable 'y' not yet declared
        int y;
    }
}

class A {
    int field1;
    
    public int method1(int a, int b) {
        int local1;
        local1 = a + b;
        return local1;
    }
    
    // Error 3: Return type mismatch
    public int method2() {
        return true;  // Returns boolean but declared as int
    }
    
    // Error 2: Method call signature mismatch
    public void method3() {
        A obj;
        obj = new A();
        int x;
        x = obj.method1(5);  // method1 needs 2 parameters, only 1 provided
        x = obj.method1(5,2,3);
        x = obj.method1("test","56");
        B tt;
        y = tt.testDuplicate("hello",5);
    }
    
    public void method4() {
        int z;
        z = this.notDeclaredMethod();  // Error 2: Method not declared
    }
}

class B {
    public void testScope() {
        int outer;
        outer = 10;
        
        if (outer > 5) {
            int inner;
            inner = 20;
        }
        
        int wrong;
        wrong = inner + 1;  // Error 5: 'inner' not accessible here
    }
    
    public void testDuplicate(int test1,String test1) {
        int var1;
        int var1;  // Error 1: Duplicate in same scope
        var1 = 5;
        return inner;  // Error 3: Return type mismatch, Error 5: 'inner' not declared
    }

    public int testDuplicate_1() {

        return ;  // Error 3: Return type mismatch
    }
    public int testDuplicate_2() {

        return "string";  // Error 3: Return type mismatch
    }
    public int testDuplicate_3() {
        String test;
        return test;  // Error 3: Return type mismatch
    }
}

// Error 4: Cyclic inheritance C->D->E->C
class C extends D {
    int data1;
}

class D extends E {
    int data2;
}

class E extends C {
    int data3;
}

// Error 4: Self-inheritance
class F extends F {
    int data;
}
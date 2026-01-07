#  پروژه کامپایلر فاز دوم - تحلیلگر معنایی (Semantic Analyzer)

**درس:** کامپایلر  
**دانشگاه:** فردوسی مشهد  
**تاریخ:** دی ماه 1404

---

## فهرست مطالب
1. [مقدمه](#مقدمه)
2. [اهداف پروژه](#اهداف-پروژه)
3. [معماری کلی سیستم](#معماری-کلی-سیستم)
4. [مراحل پیاده‌سازی](#مراحل-پیاده‌سازی)
5. [انواع خطاهای شناسایی‌شده](#انواع-خطاهای-شناسایی‌شده)
6. [ساختار داده‌ها](#ساختار-داده‌ها)
7. [نحوه اجرای برنامه](#نحوه-اجرای-برنامه)
8. [نتایج و خروجی](#نتایج-و-خروجی)
9. [نتیجه‌گیری](#نتیجه‌گیری)

---

## مقدمه

این پروژه، فاز دوم از پیاده‌سازی کامپایلر برای زبان **Java--** است که بر روی تحلیل معنایی (Semantic Analysis) تمرکز دارد. در این فاز، با استفاده از جدول نماد (Symbol Table) ایجاد شده در فاز اول، خطاهای معنایی مختلف در کدهای Java-- شناسایی می‌شوند.

### ویژگی‌های کلیدی:
- استفاده از جدول نماد برای ردیابی متغیرها، متدها و کلاس‌ها
- شناسایی پنج دسته خطای معنایی مهم
- گزارش‌دهی دقیق شماره خط وقوع خطا
- پشتیبانی از scope hierarchy و inheritance

---

## اهداف پروژه

### هدف اصلی:
پیاده‌سازی تحلیلگر معنایی که بتواند خطاهای معنایی را در زمان کامپایل شناسایی کند، قبل از اینکه کد به مرحله اجرا برسد.

### اهداف فرعی:
1. **استفاده بهینه از جدول نماد:** بهره‌برداری از اطلاعات جمع‌آوری شده در فاز اول
2. **شناسایی خطاهای رایج:** تشخیص خطاهایی که برنامه‌نویسان معمولاً مرتکب می‌شوند
3. **گزارش‌دهی واضح:** ارائه اطلاعات دقیق درباره محل و نوع خطا
4. **پشتیبانی از ویژگی‌های OOP:** مدیریت inheritance، scope و method overriding

---

## معماری کلی سیستم

سیستم از چهار بخش اصلی تشکیل شده است:

### 1. **Symbol Table (جدول نماد)**
- ذخیره‌سازی اطلاعات کلاس‌ها، متدها، متغیرها و پارامترها
- مدیریت Scope Hierarchy
- پشتیبانی از lookup در scope‌های مختلف

### 2. **SymbolCollector (جمع‌آوری نمادها)**
این کلاس که در فاز اول پیاده‌سازی شده است، وظیفه جمع‌آوری اطلاعات از parse tree را دارد و جدول نماد را می‌سازد.

#### ویژگی‌های کلیدی SymbolCollector:

**الف. مدیریت Scope:**
- استفاده از Stack برای نگهداری سلسله‌مراتب scope‌ها
- پشتیبانی از Global Scope، Class Scope، Method Scope و Block Scope
- مدیریت ویژه برای متد main و بلوک‌های داخل آن

**ب. جمع‌آوری اطلاعات:**
- **کلاس‌ها:** نام، parent class، interface‌های پیاده‌سازی شده، abstract بودن
- **متدها:** نام، نوع بازگشتی، پارامترها، access modifier، override و abstract بودن
- **متغیرها و فیلدها:** نام، نوع، مقدار اولیه، access modifier
- **Constructor‌ها:** نام، پارامترها، access modifier
- **Interface‌ها:** متدها و فیلدهای interface

**ج. ویژگی‌های خاص:**
- تشخیص annotation @Override برای متدها
- مدیریت block depth برای scope‌های تو در تو
- ذخیره شماره خط برای هر نماد (برای گزارش خطا)

**د. خروجی:**
- تولید فایل JSON با تمام نمادهای جمع‌آوری شده
- ساختار سلسله‌مراتبی scope‌ها

### 3. **SemanticInfoCollector (جمع‌آوری اطلاعات معنایی)**
جمع‌آوری اطلاعات تکمیلی برای تحلیل معنایی:
- **Variable Declarations:** تمام اعلان‌های متغیر با scope و line number
- **Method Calls:** فراخوانی متدها با آرگومان‌ها و محل فراخوانی
- **Method Declarations:** اعلان متدها با نوع بازگشتی واقعی و اعلام شده
- **Variable Usages:** استفاده از متغیرها در کد

### 4. **ErrorDetection (تشخیص خطا)**
ماژول اصلی تحلیل معنایی که با استفاده از جدول نماد و اطلاعات معنایی، خطاها را شناسایی می‌کند.

---

## مراحل پیاده‌سازی

### مرحله 1: بارگذاری داده‌ها
```
1. خواندن فایل symbols.json (جدول نماد)
2. خواندن فایل semantic_info.json (اطلاعات معنایی)
3. خواندن فایل input.txt (کد منبع)
```

### مرحله 2: ساخت جدول نماد
```
1. Parse کردن JSON symbols
2. ایجاد ساختار Scope hierarchy
3. درج نمادها در scope‌های مناسب
```

### مرحله 3: تحلیل معنایی
برای هر نوع خطا، یک تابع جداگانه اجرا می‌شود:

#### 3.1 تشخیص متغیرهای تکراری
```
- بررسی تمام اعلان‌های متغیر
- شناسایی متغیرهایی با نام یکسان در یک scope
- ثبت خطا با شماره خط دومین اعلان
```

#### 3.2 تشخیص عدم تطابق امضای متد
```
- بررسی تمام فراخوانی‌های متد
- مقایسه تعداد و نوع آرگومان‌ها با تعریف متد
- بررسی وجود متد در scope مناسب
```

#### 3.3 تشخیص عدم تطابق نوع بازگشتی
```
- مقایسه نوع اعلام شده با نوع واقعی return
- بررسی تمام دستورات return در متد
- شناسایی موارد void vs non-void
```

#### 3.4 تشخیص دور در وراثت
```
- پیمایش تمام کلاس‌ها
- بررسی زنجیره parent classes
- شناسایی circular dependencies
```

#### 3.5 تشخیص دسترسی نامعتبر به متغیر
```
- بررسی تمام استفاده‌های متغیر
- بررسی scope accessibility
- تشخیص متغیرهای استفاده شده قبل از اعلان
```

### مرحله 4: گزارش‌دهی
```
- مرتب‌سازی خطاها بر اساس شماره خط
- نمایش نوع خطا و شماره خط
- شمارش کل خطاهای یافت شده
```

---

## انواع خطاهای شناسایی‌شده

### 1. Duplicate Variable In Scope (خطای متغیر تکراری)

**توضیح:** زمانی که دو متغیر با نام یکسان در یک scope اعلان شوند.

**مثال:**
```java
public static void main(String[] args) {
    int x;
    int x;  // Error: Duplicate variable
}
```

**پیاده‌سازی:**
```cpp
vector<Error> Detect_Duplicate_Variable_In_Scope() {
    // استفاده از map برای شمارش اعلان‌ها
    unordered_map<pair<string,string>, size_t, PairHash> varCount;
    
    // بررسی تمام اعلان‌های متغیر
    for (const auto& decl : SementicData.variableDeclarations) {
        string varName = decl.at("variable_name");
        string scope = decl.at("full_scope");
        
        // اگر قبلاً در همین scope اعلان شده، خطا است
        if (varCount[{varName, scope}] > 0) {
            ans.push_back(Error(line, ErrorType::DuplicateVariableInScope));
        }
    }
}
```

---

### 2. Method Call Signature Mismatch (عدم تطابق امضای متد)

**توضیح:** تعداد یا نوع پارامترهای ارسالی با تعریف متد مطابقت ندارد.

**مثال:**
```java
public int method1(int a, int b) {
    return a + b;
}

public void method2() {
    int x = method1(5);  // Error: needs 2 parameters
    int y = method1("test", 5);  // Error: wrong types
}
```

**پیاده‌سازی:**
```cpp
vector<Error> Detect_Method_Call_Signature_Mismatch() {
    for (const auto& call : SementicData.methodCalls) {
        // یافتن تعریف متد در جدول نماد
        Symbol methodInSymbolTable = symbol_table->lookup(methodName, scope);
        
        // مقایسه تعداد پارامترها
        if (declaredParamCount != actualArgCount) {
            ans.push_back(Error(line, ErrorType::MethodCallSignatureMismatch));
        }
        
        // مقایسه نوع پارامترها
        for (size_t i = 0; i < paramCount; i++) {
            if (declaredParams[i].type.name != argTypes[i]) {
                ans.push_back(Error(line, ErrorType::MethodCallSignatureMismatch));
            }
        }
    }
}
```

---

### 3. Return Type Mismatch (عدم تطابق نوع بازگشتی)

**توضیح:** نوع مقدار بازگشتی با نوع اعلام شده در تعریف متد مطابقت ندارد.

**مثال:**
```java
public int method1() {
    return true;  // Error: returns boolean, declared as int
}

public int method2() {
    return;  // Error: returns void, declared as int
}

public void method3() {
    return 5;  // Error: returns int, declared as void
}
```

**پیاده‌سازی:**
```cpp
vector<Error> Detect_Return_Type_Mismatch() {
    for (const auto& method : SementicData.methodDeclarations) {
        string declaredReturnType = method.at("declared_return_type");
        string actualReturnType = method.at("actual_return_types");
        
        // مقایسه نوع اعلام شده با نوع واقعی
        if (declaredReturnType != actualReturnType) {
            ans.push_back(Error(returnLine, ErrorType::ReturnTypeMismatch));
        }
    }
}
```

---

### 4. Cyclic Inheritance (دور در وراثت)

**توضیح:** زمانی که زنجیره وراثت به خودش برمی‌گردد (A extends B, B extends C, C extends A).

**مثال:**
```java
class C extends D { }
class D extends E { }
class E extends C { }  // Error: Cyclic inheritance

class F extends F { }  // Error: Self-inheritance
```

**پیاده‌سازی:**
```cpp
vector<Error> Detect_Cyclic_Inheritance() {
    // پیمایش تمام کلاس‌ها
    for (each class in symbol table) {
        unordered_set<string> visited;
        string currentClass = class.name;
        
        // پیمایش زنجیره parent ها
        while (true) {
            if (visited.count(currentClass)) {
                // دور یافت شد
                ans.push_back(Error(line, ErrorType::CyclicInheritance));
                break;
            }
            visited.insert(currentClass);
            currentClass = getParent(currentClass);
        }
    }
}
```

---

### 5. Invalid Variable Access (دسترسی نامعتبر به متغیر)

**توضیح:** استفاده از متغیر در scope‌هایی که به آن دسترسی ندارند یا استفاده قبل از اعلان.

**مثال:**
```java
public void method1() {
    int x = y + 1;  // Error: y not declared yet
    int y = 5;
}

public void method2() {
    if (true) {
        int inner = 10;
    }
    int z = inner + 1;  // Error: inner not accessible here
}
```

**پیاده‌سازی:**
```cpp
vector<Error> Detect_Invalid_Variable_Access() {
    for (const auto& usage : SementicData.variableUsages) {
        string declaredAt = usage.at("declared_at");
        string usageLocation = usage.at("usage_location");
        
        // بررسی اعلان نشده بودن
        if (declaredAt == "None") {
            ans.push_back(Error(line, ErrorType::InvalidVariableAccess));
        }
        
        // بررسی scope accessibility
        if (!isParentScope(declaredAt, usageLocation)) {
            ans.push_back(Error(line, ErrorType::InvalidVariableAccess));
        }
        
        // بررسی استفاده قبل از اعلان
        if (usageLine < declaredLine) {
            ans.push_back(Error(line, ErrorType::InvalidVariableAccess));
        }
    }
}
```

---

## ساختار داده‌ها

### 1. Symbol Table Structure

```
GlobalScope
├── Class: Main
│   └── Method: main
│       ├── Variable: x
│       ├── Variable: result
│       └── Variable: y
├── Class: A
│   ├── Field: field1
│   ├── Method: method1
│   │   ├── Parameter: a
│   │   ├── Parameter: b
│   │   └── Variable: local1
│   └── Method: method2
└── Class: B
    └── Method: testScope
        ├── Variable: outer
        └── BLOCK#1
            └── Variable: inner
```

### 2. JSON Structures

#### symbols.json:
```json
{
  "symbolType": "class",
  "name": "A",
  "parent": null,
  "interfaces": [],
  "isAbstract": false,
  "scope": "GLOBAL",
  "line": 12
}
```

#### semantic_info.json:
```json
{
  "1_variable_declarations": [...],
  "2_method_calls": [...],
  "3_method_declarations": [...],
  "4_variable_usages": [...]
}
```

### 3. Error Structure

```cpp
class Error {
    size_t error_line;        // شماره خط خطا
    ErrorType error_type;     // نوع خطا
    
    void PrintError() {
        // نمایش اطلاعات خطا
    }
}
```

---

## نحوه اجرای برنامه

### پیش‌نیازها:
1. کامپایلر C++ (با پشتیبانی C++17)
2. فایل‌های ورودی:
   - `input.txt`: کد منبع Java--
   - `symbols.json`: جدول نماد (از فاز اول)
   - `semantic_info.json`: اطلاعات معنایی

### مراحل کامپایل و اجرا:

#### در Windows:
```bash
# کامپایل
g++ -std=c++17 -o compiler compiler_phase2.cpp

# اجرا
./compiler.exe
```

#### در Linux/Mac:
```bash
# کامپایل
g++ -std=c++17 -o compiler compiler_phase2.cpp

# اجرا
./compiler
```

### فلوچارت اجرا:

```
START
  ↓
Load symbols.json → Build Symbol Table
  ↓
Load semantic_info.json
  ↓
Load input.txt (source code)
  ↓
Run Error Detection:
  ├→ Detect Duplicate Variables
  ├→ Detect Method Signature Mismatch
  ├→ Detect Return Type Mismatch
  ├→ Detect Cyclic Inheritance
  └→ Detect Invalid Variable Access
  ↓
Sort Errors by Line Number
  ↓
Print All Detected Errors
  ↓
END
```

---

## نتایج و خروجی

### نمونه خروجی برنامه:

```
========================================================================
    Java-- Compiler - Error Detection
========================================================================

Loading symbol table from symbols.json...
[Info] Parsed 44 symbols from JSON
[Info] Successfully loaded 44 symbols from JSON file

==================== Symbol Table ====================
Index Name                 Kind            Type            Scope                     Initial Value  
----------------------------------------------------------------------------------------------------
0     Main                class           N/A             GLOBAL                    N/A            
1     main                method          void            Main                      N/A            
2     args                parameter       String[]        Main::main                N/A            
3     x                   variable        int             Main::main                null           
...

==================== Detected Errors ====================

[Error] Error happened in line 4
Error Type is {  Duplicate Variable In Scope  }

[Error] Error happened in line 7
Error Type is {  Invalid Variable Access  }

[Error] Error happened in line 23
Error Type is {  Return Type Mismatch  }

[Error] Error happened in line 31
Error Type is {  Method Call Signature Mismatch  }

...

=========================================================

Error count : 15

Thank you for using Java-- Compiler!
```

### آمار خطاها در فایل تست:

| نوع خطا | تعداد |
|---------|-------|
| Duplicate Variable In Scope | 4 |
| Method Call Signature Mismatch | 5 |
| Return Type Mismatch | 4 |
| Cyclic Inheritance | 2 |
| Invalid Variable Access | 4 |
| **جمع کل** | **19** |

---

## ویژگی‌های پیشرفته

### 1. Scope Hierarchy Management
- پشتیبانی از scope‌های تو در تو
- مدیریت block scopes در if، while، for
- دسترسی به متغیرهای parent scope

### 2. Type Inference
- استنتاج نوع expressions
- پشتیبانی از literals (int, boolean, String, char)
- شناسایی نوع از روی عملگرها

### 3. Inheritance Tracking
- ردیابی زنجیره وراثت
- شناسایی circular dependencies
- پشتیبانی از چند سطح وراثت

### 4. Method Resolution
- جستجوی متدها در scope‌های مختلف
- مقایسه دقیق امضاها
- پشتیبانی از overloading

---

## چالش‌ها و راه‌حل‌ها

### چالش 1: مدیریت Scope پیچیده
**مشکل:** Scope‌های تو در تو و بلوک‌های شرطی

**راه‌حل:** استفاده از Stack و نام‌گذاری منحصر به فرد (BLOCK#1, BLOCK#2)

### چالش 2: Return Type Detection
**مشکل:** شناسایی نوع return در statements مختلف

**راه‌حل:** جمع‌آوری تمام return statements و استفاده از SemanticInfoCollector

### چالش 3: Variable Accessibility
**مشکل:** بررسی دسترسی به متغیرها در scope‌های مختلف

**راه‌حل:** پیاده‌سازی تابع `isParent` برای بررسی رابطه scope‌ها

### چالش 4: JSON Parsing
**مشکل:** Parse کردن JSON با ساختار پیچیده

**راه‌حل:** پیاده‌سازی SimpleJSON parser سفارشی

---

## محدودیت‌ها

1. **عدم پشتیبانی از Generic Types:** تایپ‌های generic جاوا پشتیبانی نمی‌شوند
2. **محدودیت در Polymorphism:** بررسی method overriding کامل نیست
3. **Type Casting:** عملیات cast به طور کامل بررسی نمی‌شود
4. **Exception Handling:** خطاهای مربوط به exception ها بررسی نمی‌شوند

---

## پیشنهادات برای توسعه

### فاز بعدی:
1. **Code Generation:** تولید کد میانی یا کد ماشین
2. **Optimization:** بهینه‌سازی کد تولید شده
3. **Advanced Type Checking:** بررسی دقیق‌تر تایپ‌ها
4. **Better Error Messages:** پیام‌های خطای واضح‌تر با پیشنهاد اصلاح

### بهبودهای ممکن:
- افزودن پشتیبانی از Generic Types
- بررسی کامل‌تر method overriding
- شناسایی dead code
- بررسی null pointer exceptions
- پشتیبانی از lambda expressions

---

## نتیجه‌گیری

در این فاز از پروژه، یک تحلیلگر معنایی کامل برای زبان Java-- پیاده‌سازی شد که قادر است پنج دسته خطای معنایی مهم را شناسایی کند:

### دستاوردها:
✅ پیاده‌سازی موفق جدول نماد با پشتیبانی از Scope Hierarchy  
✅ شناسایی دقیق خطاهای معنایی با گزارش شماره خط  
✅ مدیریت صحیح وراثت و تشخیص دورهای وراثتی  
✅ بررسی امضای متدها و تطابق تایپ‌ها  
✅ تشخیص دسترسی نامعتبر به متغیرها  

### نکات کلیدی:
- استفاده از ساختارهای داده مناسب (Stack, Map) برای مدیریت Scope
- جدا کردن مسئولیت‌ها: SymbolCollector، SemanticInfoCollector، ErrorDetection
- استفاده از JSON برای تبادل داده بین فازها
- پیاده‌سازی modular و قابل توسعه

این پروژه نشان می‌دهد که چگونه یک کامپایلر می‌تواند با استفاده از جدول نماد و تحلیل دقیق، خطاهای معنایی را قبل از اجرای برنامه شناسایی کند و به برنامه‌نویس کمک کند تا کد بهتری بنویسد.

---
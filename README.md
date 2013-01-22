# Qt-based object-relational model

This project provides an object relational model that can be used with Qt and QtSql. Its goals are to be easy to use, easy to maintaing, and not too slow (fast queries must be possible to generate).

## Comparison with other ORMs

The web-development world is full of ORMs in many languages, mainly scripting ones. The [Django](http://www.djangoproject.org) web framework has one of my preferred ORM, simple yet very powerful.

In C++, there are some small or big ORMs out there. Most of them come with their own database access layer, and support a limited number of database engines. [Wt](http://www.webtoolkit.eu) provides quite a powerful ORM, that stores data in standard C++ containers and fields. For instance, an integer is stored into a `int`, a string into a `std::string`. The ORM comes with a set of serialization methods, that can be extended by the user by the use of template specialisation. SQLite, PostreSQL and MySQL are supported (the MySQL plugin is recent).

[ODB](http://www.codesynthesis.com/products/odb/) is a GCC plugin that enables the developer to annotate its C++ files with pragmas, telling how to serialize a class into a database. This ORM seems to be fast and quite simple to use, but its plugin architecture doesn't allow it to be used with Clang or any other compiler. It supports a large set of database engines.

[QxORM](http://www.qxorm.com/qxorm_en/home.html) is another Qt-based ORM. It trades ease of use for elegance, and uses nice templated functions to serialize objects. It is non intrusive and doesn't require the serialized objects to inherit from a common superclass. You can find a list of other C++ ORMs at the bottom of the home-page of this project.

### Disatvantages of QtORM

The previous section provided a list of ORMs that can be compared with QtORM. QtORM's goal is to be the closest possible to Django, while being easy to use. This goal required some tradeoffs in the design of QtORM, the most prominent one is that all QtORM models must inherit from QModel (a fairly big class), and that fields do have quite a big overhead. Don't store an array of QtORM models !

In fact, this disadvantage is mitigated by the fact that QtORM models are not really "models" but visitors. No QtORM method will return a list of QModel objects. A model is used to iterate through database objects, a signle instance of the model taking the values of every row returned by the database, one after the other.

The other C++ ORMs like to be light on resources and non-intrusive. Some of them don't require all your classes to inherit from a comon ancestor, and the majority directly use C++ primary types or STL classes to store database values. All the heavy work is done by serialization functions.

## Main features of QtORM

The memory usage of QtORM and the fact it needs you to design your objects especially as models is needed in order to provide a number of features not found in other C++ ORMs, but more common in ORMs based on a dynamic language like Python or Ruby. In fact, some of the weight of QtORM serves two purposes :

* Exposing many **compile-time checks** : the user must never have to enter a database field by its name, nor any SQL statement or part of statement. The Wt ORM, for instance, requires the user to enter SQL statements with placeholders. If your database changes name or structure, you are good for a complete re-testing of your application, and many search/replace operations.
* Using the most **C++ features** as possible, transforming it into a domain-specific language.

### Basic classes

As QtORM isn't yet documented, this feature list is accompanied by examples and pointers to classes to use.

Every database table that you want to use with QtORM must be described by a model. A model is a structure that inherits from QModel. This structure contains fields (QField members). You can add methods to these structures like in any other one, and attributes that will be populated by methods or by the constructor. QtORM will never touch any attribute that is not a QField.

```cpp
struct Model : public QModel
{
    Model();

    QStringField name;
    QIntField age;
};
```

This example show the QField subclasses used. They are `QIntField` for signed integers, `QStringField` for strings, `QDateTimeField` for date-time objects (QDateTime) and `QForeignKey<T>` for foreign keys (a pupil has a teacher, a book has an author, etc).

The constructor of this class has to initialize the fields. By default, they are invalid fields (internally, a QField subclass is simply a pointer to a QFieldPrivate subclass, so default-initializing them is fast, the pointer is simply set to NULL and isValid() returns false).

```cpp
Model::Model() : QModel("table_name")
{
    name = stringField("name");
    name.setAcceptsNull(false);
    age = intField("age");

    init();
}
```

**Don't forget to call init() !**, or your model will not be correctly "tuned" after the fields are created. This method prepends an `id` field to your fields (access it by using QModel::pk()), except if you have already provided one by using QField::setPrimaryKey().

When you have a model, you can use it like any other C++ object. The QField subclasses provide overriden operators for assignation and casting, so you can do that :

```cpp
Model m;

m.name = "Me";  // Assigns a QString
m.age = 99;     // Assigns an int
m.save();

qDebug() << "Got id" << m.pk().data();
```

The last line is needed because different database engines (or even databases of a same engines) can use different types of primary key. Some use integers, others strings, or even UUIDs. The `data()` method called returns a QVariant. You can cast it to an int with `QVariant::toInt` if you know that your database uses signed 32-bit integers. In fact, the QVariant returned is exactly the one returned by `QSqlQuery::value()`, so you can use this method if QtORM doesn't provide a field type you need, but QVariant can represent it.

Calling save another time will not create a new row in the database, but will update the one matching the current `id`.

### Performing queries

Queries are performed by the powerful QQuerySet and assorted classes. Here are the most prominent features of QtORM (saving objects can be done any other ORM). QQuerySet can :

* Select row based on complex filters, built using C++ operator overloads and without any string constant.
* Follow foreign keys
* Update rows using constant values or values of existing columns. For example, you can generate a SQL query that will add 3 to the `count` field of each row matched.
* As QtSQL can be quite slow at deserializing QVariants, you can explicitely exclude fields from the select, or select only those your are interested about.
* Rows can be batch-deleted (but it is not possible to use foreign keys in filters here, as many database engines don't support multi-table deletes)

The syntax is simple and plain C++, so here is a demonstration of all the capabilities (QF is a wrapper class around a QField, and that "escapes" it) :

```cpp
// Select all the pupils in school "A"
Pupil p;
QQuerySet q(&p);

q.addSelectRelated(p.best_friend);    // Jointure (LEFT JOIN if professor can be NULL, INNER JOIN otherwise)
q.addSelectRelated(p.class->teacher); // Use -> to dereference foreign keys. This line selects p.class and p.class->teacher
q.addFilter(QF(p.school->name) == QString("A") &&
            QF(p.present) == 1);      // Complex filters without any field name hard-coded. You can use parenthesis to nest filters, and boolean operators &&, || and !
q.excludeField(p.biography);          // Don't deserialize a big field
q.addField(p.age);                    // Explicitely add a field (it is unneeded in this example)
q.addFields(p.class);                 // Explicitely add every field of the model pointed to by a foreign key
q.addOrderBy(p.age, true);            // Ascending sort order by age
q.setLimit(10);
q.setOffset(0);                       // LIMIT and OFFSET sql clauses (yes, you can paginate with QtORM!)

while (q.next())
{
    // QQuerySet::next is a method that advances the row pointer to the next row, and populate the model and all its
    // addSelectRelated'ed foreign keys. If there is no more data to fetch, this method returns false
    qDebug() << p.name << p.best_friend->name << p.class->teacher->name << p.age;
}

// Add 1 to the age of every pupil
Pupil p;
QQuerySet u(&p);

p.age = QF(p.age) + 1;    // Use QF to refer to the "in-database" value of a field

u.update();               // UPDATE statement, with no filters. Use addFilter to add filters
```

Multiple filters are ANDed, so the addFilter call of the first example can be rewritten in two addFilter calls.

/*
We expect execution of a GridPACK application to be determined in part by a single input configuration file (perhaps extracted from a database or web service). 
We assume that a configuration file is a binding of a hierarchically structured set of keys to a set of values.  The Configuration module is used to access values
and abstract the structure of the configuration file which might be concretely represented in a ways including XML or with a custom parser. Different components 
within GridPACK will share a common configuration file and we expect then the top-level of the key name space to identify the module that is primarily associated 
with a configuration parameter. This provides extensibility over time and insulates modules from each other�s internals. 

Keys are represented by std::string with the character Configuration::KeySeparator,�.� (properly escaped in literals) used as a name specifier. So �foo.bar� is 
a key associated by convention with module �bar�.

Values are typed and may be of the following primitive types: bool, int, double or std::string. We also allow a value to be a std::vector<double>. 

We use the term cursor to a common prefix on a set of names. Access to a value may be specified by a complete key or by a cursor and the corresponding suffix 
relative to that cursor. Thus a cursor may represent the prefix �foo� and relative to that cursor the string �bar� selects the same values as �foo.bar�.  Aside 
from  factoring out common information for the client, it allows a common configuration substructure do be used by different modules.


Sample Usage:

	string src = "input.xml";
	Configuration config;
	config.enable_logging(&std::cout);
	if(config.open(src)) {
		// select a value by path name, with default
		string start = config.get("Configuration.Time.Start", "DefaultStart");
		cout << "Start " << start << endl;

		// select a subtree by path 
		Configuration::Cursor * time = config.get_cursor("Configuration.Time");

		// select a value without specified default
		double step = -1.0;
		if(time->get("Step", & step)) 
    	  	cout << "Step " << step << endl;

		std::vector<double> dv ;
		if(config.get("Configuration.Option.DefaultVelocity", &dv)) {
			cout << "DefaultVelocity: ";
			for(double e : dv) 
				cout << e << " ";
			cout << endl;
		}
	}

*/
#pragma once
#ifndef Configuration_H
// TODO -- any convention on version numbers? 
// TODO -- any cooding conventions? 
#define Configuration_H 201307
#include <string>
#include <vector>
#ifdef USE_MPI
#include <mpi.h>
#endif
namespace gridpack { namespace utilities {

class Configuration {
	class ConfigInternals * pimpl;
public:
	typedef std::string KeyType;
	static const char KeySep = '.';  // inhereted from boost, could change at some cost
	
	Configuration(void);
	~Configuration(void);

	// enable logging for diagnostics and provenence (default is std::cout)
	void enable_logging(std::ostream * = NULL);

	// read a configuration file. true == success, false == some kind of failure
#ifdef USE_MPI
	bool open(std::string file,MPI_Comm);  // rank 0 only
	bool initialize(MPI_Comm);			   // all other ranks
#else
	bool open(std::string file);  // rank 0 only
#endif
	/* 
	 *  for each supported type, there are two variants. One takes a default value 
	 *     that is returned if the key is not present in the configuration file 
	 *     the other takes a pointer to an output location and returns a boolean. When the boolean is true
	 *        the output location is updated with the value
	 */
	bool get(KeyType, bool default_value);
	bool get(KeyType, bool *);
	int get(KeyType, int default_value);
	bool get(KeyType, int *);
	double get(KeyType, double default_value);
	bool get(KeyType, double *);
	std::string get(KeyType, const std::string & default_value);
	// this wrapper makes a common case look a little cleaner 
	std::string get(KeyType key, const char * default_value) { return get(key, std::string(default_value)); }
	bool get(KeyType, std::string *);
	
	// first implementation only works with 3-vectors (which are also individually labeled X,Y,Z)
	std::vector<double> get(KeyType, const std::vector<double> & default_value);
	bool get(KeyType, std::vector<double>*);

	/*
	 * this class represents a prefix of a set of key names
	 * conveniently this implementation allows it to be the same class
	 */
	typedef Configuration Cursor ;

	/* select a prefix, returns NULL of the prefix has no defined keys */
	/* 
	 * the XML based implementation on top of boost property maps might have multiple elements with the same name
	 * this will simply return a pointer to the first which might not be sufficient.
	 */
	Cursor * get_cursor(KeyType);

	/*
  	 * For the root note, return an empty string
	 *   for a cursor, return the key-path as a string that selects this point in the heirarchy
	 */
	const std::string & path();
};


} /* utitilities */ 
} /* gridpack */
#endif

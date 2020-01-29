#include "argon2.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <list>
#include <fstream>
#include "PasswdMgr.h"
#include "FileDesc.h"
#include "strfuncts.h"

const int hashlen = 32;
const int saltlen = 16;

PasswdMgr::PasswdMgr(const char *pwd_file):_pwd_file(pwd_file) {

}


PasswdMgr::~PasswdMgr() {

}

/*******************************************************************************************
 * checkUser - Checks the password file to see if the given user is listed
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            reading
 *******************************************************************************************/

bool PasswdMgr::checkUser(const char *name) {
   std::vector<uint8_t> hash, salt;

    FileFD pwfile(_pwd_file.c_str());

    // You may need to change this code for your specific implementation

    if (!pwfile.openFile(FileFD::readfd))
        throw pwfile_error("Could not open passwd file for reading");

    while (1) {
        std::string uname;

        if (!readUser(pwfile, uname, hash, salt)) {
            return false;
        }

        if (0 == uname.compare(std::string(name))) {
            pwfile.closeFD();
            return true;
        }
    }
}

/*******************************************************************************************
 * checkPasswd - Checks the password for a given user to see if it matches the password
 *               in the passwd file
 *
 *    Params:  name - username string to check (case insensitive)
 *             passwd - password string to hash and compare (case sensitive)
 *    
 *    Returns: true if correct password was given, false otherwise
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            reading
 *******************************************************************************************/

bool PasswdMgr::checkPasswd(const char *name, const char *passwd) {
   std::vector<uint8_t> userhash; // hash from the password file
   std::vector<uint8_t> passhash; // hash derived from the parameter passwd
   std::vector<uint8_t> salt;
   std::vector<uint8_t> usersalt;

   // Check if the user exists and get the passwd string
   if (!findUser(name, userhash, salt))
      return false;

   hashArgon2(passhash, usersalt, passwd, &salt);

   if (userhash == passhash){
       userhash.clear();
       passhash.clear();
       salt.clear();
       usersalt.clear();
       return true;
   }
    userhash.clear();
    passhash.clear();
    salt.clear();
    usersalt.clear();
    return false;
}

/*******************************************************************************************
 * changePasswd - Changes the password for the given user to the password string given
 *
 *    Params:  name - username string to change (case insensitive)
 *             passwd - the new password (case sensitive)
 *
 *    Returns: true if successful, false if the user was not found
 *
 *    Throws: pwfile_error if there were unanticipated problems opening the password file for
 *            writing
 *
 *******************************************************************************************/

bool PasswdMgr::changePasswd(const char *name, const char *passwd) {
    bool result = false;
    std::vector<uint8_t> hash; // hash from the password file
    std::vector<uint8_t> salt;
    FileFD pwfile(_pwd_file.c_str());
    FileFD writer = FileFD ("temp");

    if (!writer.openFile(FileFD::writefd)){
        throw pwfile_error("could not open file for writing");
    }

    if (!pwfile.openFile(FileFD::readfd))
        throw pwfile_error("Could not open passwd file for reading");


    bool eof = false;
    while (!eof) {
        std::string uname;

        if (!readUser(pwfile, uname, hash, salt)) {
            eof = true;
        }

        if (0 == uname.compare(name)) {
            hashArgon2(hash, salt, passwd);
            writeUser(writer, (std::string &) name, hash, salt);
        }
        else{
            writeUser(writer, (std::string &) name, hash, salt);
            result = true;
        }
    }

    hash.clear();
    salt.clear();
    pwfile.closeFD();
    writer.closeFD();
    return result;
}

/*****************************************************************************************************
 * readUser - Taking in an opened File Descriptor of the password file, reads in a user entry and
 *            loads the passed in variables
 *
 *    Params:  pwfile - FileDesc of password file already opened for reading
 *             name - std string to store the name read in
 *             hash, salt - vectors to store the read-in hash and salt respectively
 *
 *    Returns: true if a new entry was read, false if eof reached 
 * 
 *    Throws: pwfile_error exception if the file appeared corrupted
 *
 *****************************************************************************************************/

bool PasswdMgr::readUser(FileFD &pwfile, std::string &name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt)
{
   // Insert your perfect code here!
   std::string bufextra;

   if (pwfile.readStr(name) <= 0) return false;
   if (pwfile.readBytes(hash, 32) <= 0) return false;
   if (pwfile.readBytes(salt, 16) <= 0) return false;
   pwfile.readStr(bufextra);

   return true;
}

/*****************************************************************************************************
 * writeUser - Taking in an opened File Descriptor of the password file, writes a user entry to disk
 *
 *    Params:  pwfile - FileDesc of password file already opened for writing
 *             name - std string of the name 
 *             hash, salt - vectors of the hash and salt to write to disk
 *
 *    Returns: bytes written
 *
 *    Throws: pwfile_error exception if the writes fail
 *
 *****************************************************************************************************/

int PasswdMgr::writeUser(FileFD &pwfile, std::string name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt)
{
   int results = 0;
   std::string str;
   auto hashstr = std::string(reinterpret_cast<char*>(&hash[0]), hash.size());
   auto saltstr = std::string(reinterpret_cast<char*>(&salt[0]), salt.size());

    str.append(name);
    str.append("\n");
    str.append(hashstr);
    str.append(saltstr);
    str.append("\n");

   // Insert your wild code here!
   results = pwfile.writeFD(str);

   return results; 
}

/*****************************************************************************************************
 * findUser - Reads in the password file, finding the user (if they exist) and populating the two
 *            passed in vectors with their hash and salt
 *
 *    Params:  name - the username to search for
 *             hash - vector to store the user's password hash
 *             salt - vector to store the user's salt string
 *
 *    Returns: true if found, false if not
 *
 *    Throws: pwfile_error exception if the pwfile could not be opened for reading
 *
 *****************************************************************************************************/

bool PasswdMgr::findUser(const char *name, std::vector<uint8_t> &hash, std::vector<uint8_t> &salt) {

   FileFD pwfile(_pwd_file.c_str());

   // You may need to change this code for your specific implementation

   if (!pwfile.openFile(FileFD::readfd))
      throw pwfile_error("Could not open passwd file for reading");

   // Password file should be in the format username\n{32 byte hash}{16 byte salt}\n
   bool eof = false;
   while (!eof) {
      std::string uname;

      if (!readUser(pwfile, uname, hash, salt)) {
         eof = true;
         continue;
      }

      if (!uname.compare(name)) {
         pwfile.closeFD();
         return true;
      }
   }

   hash.clear();
   salt.clear();
   pwfile.closeFD();
   return false;
}


/*****************************************************************************************************
 * hashArgon2 - Performs a hash on the password using the Argon2 library. Implementation algorithm
 *              taken from the http://github.com/P-H-C/phc-winner-argon2 example. 
 *
 *    Params:  dest - the std string object to store the hash
 *             passwd - the password to be hashed
 *
 *    Throws: runtime_error if the salt passed in is not the right size
 *****************************************************************************************************/
void PasswdMgr::hashArgon2(std::vector<uint8_t> &ret_hash, std::vector<uint8_t> &ret_salt,
                           const char *in_passwd, std::vector<uint8_t> *in_salt) {
    uint8_t hash[hashlen];
    uint8_t salt[saltlen];
    srand(time(0));
    if (in_salt!=NULL){
        if (in_salt->size() != saltlen){
            throw std::runtime_error("salt length incorrect");
        }
        for (int i = 0; i < saltlen; i++) salt[i] = (*in_salt)[i];

    }else{
    for (int i = 0; i < saltlen; i++) salt[i] = ((rand() % 33)+33);
    }

    uint32_t t_cost = 2;            // 1-pass computation
    uint32_t m_cost = (1<<16);      // 64 mebibytes memory usage
    uint32_t parallelism = 1;       // number of threads and lanes

    // high-level API
    argon2i_hash_raw(t_cost, m_cost, parallelism, &in_passwd, strlen(in_passwd), salt, 16, hash,
                     32);
    ret_hash.clear();
    ret_hash.reserve(hashlen);
    ret_salt.clear();
    ret_salt.reserve(saltlen);
    for (int i = 0; i < hashlen; i++)
        ret_hash.push_back(hash[i]);
    for (int i = 0; i< saltlen; i++)
        ret_salt.push_back(salt[i]);

}

/****************************************************************************************************
 * addUser - First, confirms the user doesn't exist. If not found, then adds the new user with a new
 *           password and salt
 *
 *    Throws: pwfile_error if issues editing the password file
 ****************************************************************************************************/

void PasswdMgr::addUser(const char *name, const char *passwd) {
   // Add those users!
    std::vector<uint8_t> hash, salt; // hash from the password file
    FileFD pwfile(_pwd_file.c_str());
    std::ofstream outfile ("temp.txt");
    FileFD writefile ("temp.txt");

    if (!writefile.openFile(FileFD::writefd))
        throw pwfile_error("Could not open write file for writing");

    // You may need to change this code for your specific implementation

    if (!pwfile.openFile(FileFD::readfd))
        throw pwfile_error("Could not open passwd file for reading");

    // Password file should be in the format username\n{32 byte hash}{16 byte salt}\n
    bool eof = false;
    while (!eof) {
        std::string uname;
        if (!readUser(pwfile, uname, hash, salt)) {
            eof = true;
            hashArgon2(hash, salt, passwd);
            writeUser(writefile, (std::string ) name, hash, salt);
        }

        else if (0 == uname.compare(std::string(name))) {
            throw pwfile_error("user already exists");
        }
        else{
            writeUser(writefile, (std::string ) uname, hash, salt);
        }
    }

    rename("temp.txt", _pwd_file.c_str());
    writefile.closeFD();
    pwfile.closeFD();
    hash.clear();
    salt.clear();
}


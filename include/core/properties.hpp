/**
 * @file properties.hpp
 *
 * @brief Property Map Implementation for Algorithms
 */

#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

#include <any>
#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace syrec {

    /**
   * @brief Property Map for storing settings and statistical information
   *
   * In this data structure settings and statistical data can be stored.
   * The key to access data is always of type \p std::string and the value
   * can be of any type. To be type-safe, the getter corresponding get
   * functions have to be provided with a type.
   */
    struct properties {
        /**
     * @brief Internal storage type used with the internal property map
     */
        typedef std::map<std::string, std::any> storage_type;

        /**
     * @brief Value type of the property map, i.e. \p std::string
     */
        typedef storage_type::mapped_type value_type;

        /**
     *
     * There are pre-defined getter methods, which can be called with a
     * type identifier for explicit casting.
     */
        typedef storage_type::key_type key_type;

        /**
     * @brief Smart Pointer version of this class
     *
     * Inside the framework, always the Smart Pointer version is used.
     * To have an easy access, there are special functions provided
     * which take the smart pointer as parameter and check as well
     * if it can be dereferenced.
     *
     * @sa get
     * @sa set_error_message
     */
        typedef std::shared_ptr<properties> ptr;

        /**
     * @brief Standard constructor
     *
     * Creates the property map on base of the storage map
     */
        properties() = default;

        /**
     * @brief Casted access to an existing element
     *
     * With \p T you can specify the type of the element. Note, that
     * it has to be the original used type, e.g. there is a difference
     * even between \p int and \p unsigned.
     *
     * The type is determined automatically using the set method.
     *
     * @param k Key to access the property map. Must exist.
     * @return The value associated with key \p k casted to its original type \p T.
     */
        template<typename T>
        T get(const std::string& k) const {
            return std::any_cast<T>(map.find(k)->second);
        }

        /**
     * @brief Casted access to an existing element with fall-back option
     *
     * The same as get(const key_type& k), but if \p k does not exist,
     * a default value is returned, which has to be of type \p T.
     *
     * @param k Key to access the property map. May not exist.
     * @param default_value If \p k does not exist, this value is returned.
     * @return The value associated with key \p k casted to its original type \p T. If the key \p k does not exist,
     *         \p default_value is returned.
     */
        template<typename T>
        T get(const key_type& k, const T& default_value) const {
            if (map.find(k) == map.end()) {
                return default_value;
            } else {
                return std::any_cast<T>(map.find(k)->second);
            }
        }

        /**
     * @brief Adds or modifies a value in the property map
     *
     * This methods sets the value located at key \p k to \p value.
     * If the key does not exist, it will be created.
     * Be careful which type was used, especially with typed constants:
     * @code
     * properties p;
     * p.set( "a unsigned number", 5u );
     * p.get<unsigned>( "a unsigned number" ); // OK!
     * p.get<int>( "a unsigned number" );      // FAIL!
     *
     * p.set( "a signed number", 5 );
     * p.get<unsigned>( "a signed number" );   // FAIL!
     * p.get<int>( "a signed number" );        // OK!
     * @endcode
     *
     * @param k Key of the property
     * @param value The new value of \p k. If \p k already existed, the type of \p value must not change.
     */

        template<typename T>
        void set(const std::string& k, const T& value) {
            map[k] = value;
        }

    private:
        storage_type map;
    };

    /**
   * @brief A helper method to access the get method on a properties smart pointer
   *
   * This method has basically two fall backs. If settings does not point to anything,
   * it returns \p default_value, and otherwise it calls the get method on the
   * pointee of the smart pointer with the \p default_value again, so in case the key \p k
   * does not exists, the \p default_value is returned as well.
   *
   * @param settings A smart pointer to a properties instance or an empty smart pointer
   * @param k Key of the property to be accessed
   * @param default_value A default_value as fall back option in case the smart pointer
   *                      is empty or the key does not exist.
   *
   * @return The value addressed by \p k or the \p default_value.
   */
    template<typename T>
    T get(const properties::ptr& settings, const properties::key_type& k, const T& default_value) {
        return settings ? settings->get<T>(k, default_value) : default_value;
    }

} // namespace syrec

#endif /* PROPERTIES_HPP */

#ifndef RESOURCEPOOL_HPP
#define RESOURCEPOOL_HPP

#include <map>

template<class Key, class Value>
class ResourcePool
{
public:
	Value *get(Key k);
	void preload(Key k);
	void reload(Key k);
	void clear();
	void reload();
	
protected:
	typedef std::map<Key,Value*> Pool;
	Pool contents;
};

template<class Key, class Value>
Value *ResourcePool<Key,Value>::get(Key k)
{
	Pool::iterator ii=contents.find(k);
	if(ii==contents.end()) {
		contents[k] = NEW Value(k);
		return contents[k];
	}
	return ii->second;
}
template<class Key, class Value>
void ResourcePool<Key,Value>::preload(Key k)
{
	get(k);
}
template<class Key, class Value>
void ResourcePool<Key,Value>::reload(Key k)
{
	Pool::iterator ii=contents.find(k);
	if(ii==contents.end())
		return (contents[k]=NEW Value(k));
	delete(ii->second);
	ii->second = NEW Value(k);
}
template<class Key, class Value>
void ResourcePool<Key,Value>::clear()
{
	for(Pool::iterator ii=contents.begin(); ii!=contents.end(); ii++)
		delete(ii->second);
	contents.clear();
}
template<class Key, class Value>
void ResourcePool<Key,Value>::reload() {
	for(Pool::iterator ii=contents.begin(); ii!=contents.end(); ii++) {
		delete(ii->second);
		ii->second = NEW Value(ii->first);
	}
}


#endif
#pragma once

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "PltStateVariable.h"
#include "PltMediaItem.h"
#include "PltSyncPolicy.h"
#include "PltUtilities.h"
#include "PltSyncStatus.h"

bool IsSyncable(const NPT_String& object_class);
bool IsValidUDN(const NPT_String& udn);
bool IsValidID(const NPT_String& id);
bool TranslateErrorCode(const NPT_UInt32 errorCode, char** errorTitle, char** errorDescription);

class PLT_SyncStructure;
struct PLT_Partner;

struct DeviceUDNFinder
{
  DeviceUDNFinder(const NPT_String& udn) : udn(udn) {}
  bool operator()(const PLT_Partner& partner) const;
  const NPT_String& udn;
};

template<typename T, template <typename> class C>
class CopyListCast
{
public:
  CopyListCast(C<T>& container) : container(container) {}

  template<typename ORIG>
  NPT_Result operator()(const ORIG& orig) const
  {
    return container.Add((T)orig);
  }

  C<T>& container;
};

struct IDFinder
{
  IDFinder(const NPT_String& id) : id(id) {}
  template<typename T>
  bool operator()(const T& obj) const
  {
    return obj->GetID() == id;
  }
  const NPT_String& id;
};


template<class T>
struct ToXMLFunctor
{
  ToXMLFunctor(NPT_String* result) : resultStr(result), resultValue(NPT_SUCCESS) {}

  NPT_String* resultStr;
  mutable NPT_Result resultValue;
  void operator()(const NPT_Reference<T>& obj) const
  {
    NPT_Result result;
    result = obj->ToXml(*resultStr);
    if (NPT_FAILED(result))
      resultValue = result;
  }
  void operator()(const T& obj) const
  {
    NPT_Result result;
    result = obj.ToXml(*resultStr);
    if (NPT_FAILED(result))
      resultValue = result;
  }

};


template<typename T, template <typename> class C >
NPT_Result PrintArrayToXml(const C<T> array, NPT_String& xml)
{
  ToXMLFunctor<T> function(&xml);
  array.Apply(function);
  return function.resultValue;
}

template<typename T, template <typename> class Ref, template <typename> class C >
NPT_Result PrintArrayToXml(const C<Ref<T> > array, NPT_String& xml)
{
  ToXMLFunctor<T> function(&xml);
  array.Apply(function);
  return function.resultValue;
}

template<typename T, template<typename> class Ref, template <typename> class C>
NPT_Result LoadArrayFromXml(const NPT_String& tag, NPT_XmlElementNode* elem, C<Ref<T> >& array)
{
  NPT_Array<NPT_XmlElementNode*> nodes;
  NPT_CHECK(PLT_XmlHelper::GetChildren(elem, nodes, tag));
  for (unsigned int i = 0; i < nodes.GetItemCount(); ++i) //reverse through the list, so we cann Add() the items at front and still have the same order.
  {
    Ref<T> object(new T);
    NPT_CHECK(object->FromXml(nodes[i]));
    array.Add(object);
  }
  return NPT_SUCCESS;
}

template<typename T, template <typename> class C>
NPT_Result LoadArrayFromXml(const NPT_String& tag, NPT_XmlElementNode* elem, C<T>& array)
{
  NPT_Array<NPT_XmlElementNode*> nodes;
  NPT_CHECK(PLT_XmlHelper::GetChildren(elem, nodes, tag));
  for (unsigned int i = 0; i < nodes.GetItemCount(); ++i) //reverse through the list, so we cann Add() the items at front and still have the same order.
  {
    T object;
    NPT_CHECK(object.FromXml(nodes[i]));
    array.Add(object);
  }
  return NPT_SUCCESS;
}


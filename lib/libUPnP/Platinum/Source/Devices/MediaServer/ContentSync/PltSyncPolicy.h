#ifndef _PLT_SYNC_POLICY_H_
#define _PLT_SYNC_POLICY_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"

struct PLT_OptionalBool
{
  enum Values {
    UNDEFINED_ = -1,
    FALSE_ = 0,
    TRUE_ = 1
  };

  PLT_OptionalBool() : m_value(UNDEFINED_) {}
  PLT_OptionalBool(bool value) : m_value(value ? TRUE_ : FALSE_) {}

  bool IsValid() const { return m_value!=UNDEFINED_; }
  bool IsTrue() const { return m_value==TRUE_; }
  bool IsFalse() const { return m_value==FALSE_; }
  void Reset() { m_value = UNDEFINED_; }
  Values GetValue() const { return m_value; }
  void SetValue(Values value) { m_value = value; }
  void SetValue(bool value) { m_value = (value ? TRUE_ : FALSE_); }
  NPT_Result fromString(const NPT_String& str)
  {
    if(str.IsEmpty())
      m_value = UNDEFINED_;
    else if (str=="0" || str.Compare("false", true))
      m_value = FALSE_;
    else if (str=="1" || str.Compare("true", true))
      m_value = TRUE_;
    else {
      m_value = UNDEFINED_;
      return NPT_ERROR_INVALID_PARAMETERS;
    }
    return NPT_SUCCESS;
  }
  NPT_String toString() const {
    switch (m_value) {
      case TRUE_:
        return "1";
      case FALSE_:
        return "0";
      case UNDEFINED_:
      default:
        return "";
    }
  }

  bool operator ==(const PLT_OptionalBool& rhs) const
  {
    return m_value == rhs.m_value;
  }
  bool operator !=(const PLT_OptionalBool& rhs) const
  {
    return m_value != rhs.m_value;
  }

protected:

  Values m_value;
};

class PLT_SyncPolicy
{
public:
  PLT_SyncPolicy() : m_priorityPartnerID(0) {}

  const NPT_String& GetPolicyType() const { return m_policyType; }
  bool IsPriorityParnter(const NPT_UInt32 id) const { return m_priorityPartnerID == id;  }
  bool IsComplete()
  {
    if (!m_delProtection.IsValid() ||
        !m_autoObjAdd.IsValid()    ||
         m_policyType.IsEmpty()    ||//We don't check if the policytype has a valid value, so applications can add their own policy type
         m_priorityPartnerID > 2)    //Only 1 or two is allowed... dependingon policyType, it can be ommited (value of 0)
      return false;

    if (m_policyType.Compare("bla", true)) //TODO: these sync policies requere a priority partner (it's optional for the others!)
    {
      if (m_priorityPartnerID == 0)
        return false;
    }
    return true;
  }
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

  bool operator==(const PLT_SyncPolicy& rhs) const
  {
    return this->m_policyType == rhs.m_policyType &&
           this->m_priorityPartnerID == rhs.m_priorityPartnerID &&
           this->m_delProtection == rhs.m_delProtection &&
           this->m_autoObjAdd == rhs.m_autoObjAdd;
  }

  NPT_String m_policyType;
  NPT_UInt32 m_priorityPartnerID;
  PLT_OptionalBool m_delProtection;
  PLT_OptionalBool m_autoObjAdd;
};

#endif //_PLT_SYNC_POLICY_H_
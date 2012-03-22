// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.3.1
// Generated from file `ptencoders.ice'

#ifndef __ptencoders_h__
#define __ptencoders_h__

#include <Ice/LocalObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/Outgoing.h>
#include <Ice/Incoming.h>
#include <Ice/Direct.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/FactoryTable.h>
#include <Ice/StreamF.h>
#include <jderobot/common.h>
#include <Ice/UndefSysMacros.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 303
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 > 50
#       error Beta header file detected
#   endif
#   if ICE_INT_VERSION % 100 < 1
#       error Ice patch level mismatch!
#   endif
#endif

namespace IceProxy
{

namespace jderobot
{

class PTEncodersData;

class PTEncoders;

}

}

namespace jderobot
{

class PTEncodersData;
bool operator==(const PTEncodersData&, const PTEncodersData&);
bool operator<(const PTEncodersData&, const PTEncodersData&);

class PTEncoders;
bool operator==(const PTEncoders&, const PTEncoders&);
bool operator<(const PTEncoders&, const PTEncoders&);

}

namespace IceInternal
{

::Ice::Object* upCast(::jderobot::PTEncodersData*);
::IceProxy::Ice::Object* upCast(::IceProxy::jderobot::PTEncodersData*);

::Ice::Object* upCast(::jderobot::PTEncoders*);
::IceProxy::Ice::Object* upCast(::IceProxy::jderobot::PTEncoders*);

}

namespace jderobot
{

typedef ::IceInternal::Handle< ::jderobot::PTEncodersData> PTEncodersDataPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::jderobot::PTEncodersData> PTEncodersDataPrx;

void __read(::IceInternal::BasicStream*, PTEncodersDataPrx&);
void __patch__PTEncodersDataPtr(void*, ::Ice::ObjectPtr&);

typedef ::IceInternal::Handle< ::jderobot::PTEncoders> PTEncodersPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::jderobot::PTEncoders> PTEncodersPrx;

void __read(::IceInternal::BasicStream*, PTEncodersPrx&);
void __patch__PTEncodersPtr(void*, ::Ice::ObjectPtr&);

}

namespace IceProxy
{

namespace jderobot
{

class PTEncodersData : virtual public ::IceProxy::Ice::Object
{
public:
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_context(const ::Ice::Context& __context) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_context(__context).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_context(__context).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_adapterId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_adapterId(__id).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_adapterId(__id).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_endpoints(const ::Ice::EndpointSeq& __endpoints) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_endpoints(__endpoints).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_endpoints(__endpoints).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_locatorCacheTimeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_locatorCacheTimeout(__timeout).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_connectionCached(bool __cached) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_connectionCached(__cached).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_connectionCached(__cached).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_endpointSelection(::Ice::EndpointSelectionType __est) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_endpointSelection(__est).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_endpointSelection(__est).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_secure(bool __secure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_secure(__secure).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_secure(__secure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_preferSecure(bool __preferSecure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_preferSecure(__preferSecure).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_preferSecure(__preferSecure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_router(const ::Ice::RouterPrx& __router) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_router(__router).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_router(__router).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_locator(const ::Ice::LocatorPrx& __locator) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_locator(__locator).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_locator(__locator).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_collocationOptimized(bool __co) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_collocationOptimized(__co).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_collocationOptimized(__co).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_twoway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_twoway().get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_twoway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_oneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_oneway().get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_oneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_batchOneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_batchOneway().get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_datagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_datagram().get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_datagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_batchDatagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_batchDatagram().get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_compress(bool __compress) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_compress(__compress).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_compress(__compress).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_timeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_timeout(__timeout).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_timeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncodersData> ice_connectionId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncodersData*>(_Base::ice_connectionId(__id).get());
    #else
        return dynamic_cast<PTEncodersData*>(::IceProxy::Ice::Object::ice_connectionId(__id).get());
    #endif
    }
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
    virtual ::IceProxy::Ice::Object* __newInstance() const;
};

class PTEncoders : virtual public ::IceProxy::Ice::Object
{
public:

    ::jderobot::PTEncodersDataPtr getPTEncodersData()
    {
        return getPTEncodersData(0);
    }
    ::jderobot::PTEncodersDataPtr getPTEncodersData(const ::Ice::Context& __ctx)
    {
        return getPTEncodersData(&__ctx);
    }
    
private:

    ::jderobot::PTEncodersDataPtr getPTEncodersData(const ::Ice::Context*);
    
public:
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_context(const ::Ice::Context& __context) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_context(__context).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_context(__context).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_adapterId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_adapterId(__id).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_adapterId(__id).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_endpoints(const ::Ice::EndpointSeq& __endpoints) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_endpoints(__endpoints).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_endpoints(__endpoints).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_locatorCacheTimeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_locatorCacheTimeout(__timeout).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_locatorCacheTimeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_connectionCached(bool __cached) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_connectionCached(__cached).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_connectionCached(__cached).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_endpointSelection(::Ice::EndpointSelectionType __est) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_endpointSelection(__est).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_endpointSelection(__est).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_secure(bool __secure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_secure(__secure).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_secure(__secure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_preferSecure(bool __preferSecure) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_preferSecure(__preferSecure).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_preferSecure(__preferSecure).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_router(const ::Ice::RouterPrx& __router) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_router(__router).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_router(__router).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_locator(const ::Ice::LocatorPrx& __locator) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_locator(__locator).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_locator(__locator).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_collocationOptimized(bool __co) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_collocationOptimized(__co).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_collocationOptimized(__co).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_twoway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_twoway().get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_twoway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_oneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_oneway().get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_oneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_batchOneway() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_batchOneway().get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_batchOneway().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_datagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_datagram().get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_datagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_batchDatagram() const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_batchDatagram().get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_batchDatagram().get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_compress(bool __compress) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_compress(__compress).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_compress(__compress).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_timeout(int __timeout) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_timeout(__timeout).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_timeout(__timeout).get());
    #endif
    }
    
    ::IceInternal::ProxyHandle<PTEncoders> ice_connectionId(const std::string& __id) const
    {
    #if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
        typedef ::IceProxy::Ice::Object _Base;
        return dynamic_cast<PTEncoders*>(_Base::ice_connectionId(__id).get());
    #else
        return dynamic_cast<PTEncoders*>(::IceProxy::Ice::Object::ice_connectionId(__id).get());
    #endif
    }
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
    virtual ::IceProxy::Ice::Object* __newInstance() const;
};

}

}

namespace IceDelegate
{

namespace jderobot
{

class PTEncodersData : virtual public ::IceDelegate::Ice::Object
{
public:
};

class PTEncoders : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ::jderobot::PTEncodersDataPtr getPTEncodersData(const ::Ice::Context*) = 0;
};

}

}

namespace IceDelegateM
{

namespace jderobot
{

class PTEncodersData : virtual public ::IceDelegate::jderobot::PTEncodersData,
                       virtual public ::IceDelegateM::Ice::Object
{
public:
};

class PTEncoders : virtual public ::IceDelegate::jderobot::PTEncoders,
                   virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual ::jderobot::PTEncodersDataPtr getPTEncodersData(const ::Ice::Context*);
};

}

}

namespace IceDelegateD
{

namespace jderobot
{

class PTEncodersData : virtual public ::IceDelegate::jderobot::PTEncodersData,
                       virtual public ::IceDelegateD::Ice::Object
{
public:
};

class PTEncoders : virtual public ::IceDelegate::jderobot::PTEncoders,
                   virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual ::jderobot::PTEncodersDataPtr getPTEncodersData(const ::Ice::Context*);
};

}

}

namespace jderobot
{

class PTEncodersData : virtual public ::Ice::Object
{
public:

    typedef PTEncodersDataPrx ProxyType;
    typedef PTEncodersDataPtr PointerType;
    
    PTEncodersData() {}
    PTEncodersData(::Ice::Float, ::Ice::Float, ::Ice::Int);
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();


    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);

    static const ::Ice::ObjectFactoryPtr& ice_factory();

protected:

    virtual ~PTEncodersData() {}

    friend class PTEncodersData__staticInit;

public:

    ::Ice::Float panAngle;

    ::Ice::Float tiltAngle;

    ::Ice::Int clock;
};

class PTEncodersData__staticInit
{
public:

    ::jderobot::PTEncodersData _init;
};

static PTEncodersData__staticInit _PTEncodersData_init;

class PTEncoders : virtual public ::Ice::Object
{
public:

    typedef PTEncodersPrx ProxyType;
    typedef PTEncodersPtr PointerType;
    
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::jderobot::PTEncodersDataPtr getPTEncodersData(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::Ice::DispatchStatus ___getPTEncodersData(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

}

#endif

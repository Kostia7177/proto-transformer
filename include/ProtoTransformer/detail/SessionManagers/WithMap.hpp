#pragma once

#include <map>
#include <mutex>
#include "Base.hpp"

namespace ProtoTransformer
{

struct SessionManagerWithMap
{
    template<class S> class Reference;
    template<class S> class ExitDispatcher;

    template<class S>
    class Itself
    {   // Session manager as is,  and so iit is to be:
        //    -- created by Server before it starts the accepting;
        //    -- wrapped by shared pointer to be accessible either
        //       by Server or by all Sessions via their field 'managerReference';
        class SessionPtr
        {   // intermediate object that is to be stored as data-field of a map,
            // contains reference to a target Session and lets know to it's
            // 'managerReference' that the Session is dropped if it so (by external
            // request, by Session's request or when Server's destructor called);
            std::shared_ptr<S> reference;

            public:

            SessionPtr(std::shared_ptr<S> arg) : reference(arg){}
            ~SessionPtr(){ reference->getManagerReference().markAsExiting(); }
        };

        std::shared_ptr<ExitDispatcher<S>> exitDispatcherPtr;
        std::map<Reference<S> *, std::shared_ptr<SessionPtr>> ioConnections;

        public:

        Itself() : exitDispatcherPtr(new ExitDispatcher<S>(this)){}
        ~Itself();

        template<class W> void startSession(std::shared_ptr<S>, W &);
        void drop(Reference<S> *session2Drop){ ioConnections.erase(session2Drop); }
    };

    template<class S>
    class ExitDispatcher
    {
        Itself<S> *sessionManagerPtr;
        public:
        std::mutex locker;
        ExitDispatcher(Itself<S> *arg) : sessionManagerPtr(arg){}
        void switchOff(){ sessionManagerPtr = 0; }
        void unlink(Reference<S> *);
    };

    template<class S>
    class Reference
    {   // Session-side part of a sessionManager, field 'manager' of
        // a Session, knows about session manager Itself by keeping a shared
        // pointer to ExitDispatcher (witch knows about session manager Itself through
        // a pointer to it), notifies it when Session is about exiting ('unlink()'
        // method) and accepts request from a session manager Itself to a Session
        // to exit ('markAsExiting()' method)
        enum Status { running = 0, wasRemoved = 0x1 };
        int status;
        typedef std::shared_ptr<ExitDispatcher<S>> ExitDispatcherPtr;
        ExitDispatcherPtr exitDispatcherPtr;
        unsigned long int sessionId;

        public:

        Reference()
            : status(running),
              sessionId((unsigned long int)this){}

        bool sessionWasRemoved(){ return status & wasRemoved; }
        void markAsExiting(){ status |= wasRemoved; }
        void unlink(){ exitDispatcherPtr->unlink(this); }
        unsigned long int getId(){ return sessionId; }
        void setDispatcher(ExitDispatcherPtr arg){ exitDispatcherPtr = arg; }
    };

    template<class S>
    class ExitDetector
        : public ExitDetectorBase<S>
    {
        public:
        ExitDetector(std::shared_ptr<S> arg) : ExitDetectorBase<S>(arg){}
        ~ExitDetector(){ this->getSession().getManagerReference().unlink(); }
    };
};

template<class S>
SessionManagerWithMap::Itself<S>::~Itself()
{
    std::lock_guard<std::mutex> lockWhenExiting(exitDispatcherPtr->locker);
    exitDispatcherPtr->switchOff();
    ioConnections.clear();
}

template<class S>
template<class W>
void SessionManagerWithMap::Itself<S>::startSession(
    std::shared_ptr<S> session,
    W &workflow)
{
    std::lock_guard<std::mutex> lockWhenInserting(exitDispatcherPtr->locker);
    session->getManagerReference().setDispatcher(exitDispatcherPtr);
    ioConnections.insert(std::make_pair(&session->getManagerReference(), std::shared_ptr<SessionPtr>(new SessionPtr(session))));
    // now the pointer to a session is shared between  an 'ioConnections' map...
    session->run(workflow);
    // ...and a chain of async-calls;
}

template<class S>
void SessionManagerWithMap::ExitDispatcher<S>::unlink(Reference<S> *reference)
{
    std::lock_guard<std::mutex> lockWhenUnlinking(locker);
    if (sessionManagerPtr)
    {
        sessionManagerPtr->drop(reference);
    }
}

}

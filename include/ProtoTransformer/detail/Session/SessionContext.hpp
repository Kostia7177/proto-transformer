#pragma once

template<class Cfg>
struct SessionContext
{
    typename Cfg::SessionHdr sessionHdr;            // -- session invariants, sent by
                                                    //    client after the connection
                                                    //    have been established;
    const typename Cfg::SessionHdr &sessionHdrRO;
    typename Cfg::SessionSpecific sessionSpecific;  // -- session data (alternate to
                                                    //    an invariant sessionHdr, is
                                                    //    a variable data); available
                                                    //    for each request;
                                                    //    such a request-static data;
                                                    //
    typename Cfg::InitSessionSpecific initSessionSpecific;  // user-defined function,
                                                            // that initialises session-
                                                            // specific (may be by
                                                            // session header);
    SessionContext() : sessionHdrRO(sessionHdr){}
};

//
//  MarcoTools.h
//  SplendidTunnel
//
//  Created by CmST0us on 17/6/5.
//  Copyright © 2017年 CmST0us. All rights reserved.
//

#ifndef MarcoTools_h
#define MarcoTools_h

#define REF_GETTER(x) \
    decltype(x) & get_##x() {\
        return this->x;\
    }

#define GETTER(x) \
    decltype(x) & get_##x() {\
        return this->x;\
    }

#define READ_ONLY_REF_GETTER(x) \
    decltype(x) & get_##x() const{\
        return this->x;\
    }
#define BUILDER_SETTER(b, x) \
    b & set_##x(decltype(x) v) {\
        this->x = v;\
        return *this;\
    }

#define SETTER(x)\
    void set_##x(decltype(x) v) {\
        this->x = v;\
    }

#endif /* MarcoTools_h */

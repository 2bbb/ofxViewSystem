//
//  components/type_utils.h
//
//  Created by ISHII 2bit on 2018/02/21.
//

#pragma once

#ifndef bbb_components_type_utils_hpp
#define bbb_components_type_utils_hpp

#include <type_traits>

namespace bbb {
    namespace view_system {
        inline namespace components {
            namespace type_utils {
                template <typename type, typename default_type>
                using return_type_t = typename std::conditional<
                    std::is_same<void, type>::value,
                    default_type,
                    type
                >::type;
            };
        }
    }
}

#endif /* type_utils_h */

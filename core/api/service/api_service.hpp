/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_API_SERVICE_HPP
#define KAGOME_CORE_API_SERVICE_HPP

#include <functional>

#include "api/jrpc/jrpc_server.hpp"
#include "api/transport/listener.hpp"
#include "common/logger.hpp"

namespace kagome::api {

  class JRpcProcessor;

  /**
   * Service listening for incoming JSON RPC requests
   */
  class ApiService : public std::enable_shared_from_this<ApiService> {
   public:
    template <class T>
    using sptr = std::shared_ptr<T>;

    /**
     * @brief constructor
     * @param context - reference to the io context
     * @param listener - a shared ptr to the endpoint listener instance
     * @param processors - shared ptrs to JSON processor instances
     */
    ApiService(std::shared_ptr<Listener> listener,
               std::shared_ptr<JRpcServer> server);

    virtual ~ApiService() = default;
    /**
     * @brief starts service
     */
    virtual void start();

    /**
     * @brief stops service
     */
    virtual void stop();

   private:
    sptr<Listener> listener_;
    std::shared_ptr<JRpcServer> server_;
    common::Logger logger_;
  };
}  // namespace kagome::api

#endif  // KAGOME_CORE_API_SERVICE_HPP

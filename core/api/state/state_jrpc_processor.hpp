/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_STATE_JRPC_PROCESSOR_HPP
#define KAGOME_STATE_JRPC_PROCESSOR_HPP

#include <mutex>

#include <boost/noncopyable.hpp>

#include "api/jrpc/jrpc_processor.hpp"
#include "api/jrpc/jrpc_server.hpp"
#include "api/state/state_api.hpp"

namespace kagome::api {

  class StateJrpcProcessor : public JRpcProcessor, private boost::noncopyable {
   public:
    StateJrpcProcessor(std::shared_ptr<JRpcServer> server,
                       std::shared_ptr<StateApi> api);
    ~StateJrpcProcessor() override = default;

    void registerHandlers() override;

   private:
    std::shared_ptr<StateApi> api_;
    std::shared_ptr<JRpcServer> server_;
  };

}  // namespace kagome::api
#endif  // KAGOME_STATE_JRPC_PROCESSOR_HPP

#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-strict

import abc
from enum import Enum
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from fbpmp.private_lift.entity.privatelift_instance import PrivateLiftInstance
    from fbpmp.private_lift.service.privatelift import PrivateLiftService


class PostProcessingHandlerStatus(Enum):
    UNKNOWN = "UNKNOWN"
    STARTED = "STARTED"
    COMPLETED = "COMPLETED"
    FAILED = "FAILED"


class PostProcessingHandler(abc.ABC):
    @abc.abstractmethod
    async def run(
        self, pl_service: "PrivateLiftService", pl_instance: "PrivateLiftInstance"
    ) -> None:
        raise NotImplementedError

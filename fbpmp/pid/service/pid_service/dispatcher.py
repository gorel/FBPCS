#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import abc
from typing import Any, Dict, Optional, Union

from fbpmp.pid.entity.pid_instance import PIDProtocol, PIDRole


class Dispatcher(abc.ABC):
    @abc.abstractmethod
    def build_stages(
        self,
        input_path: str,
        output_path: str,
        num_shards: int,
        pid_config: Union[Dict[str, Any], str],
        protocol: PIDProtocol,
        role: PIDRole,
        data_path: Optional[str] = None,
        spine_path: Optional[str] = None,
    ) -> None:
        pass

    @abc.abstractmethod
    async def run_all(
        self,
    ) -> None:
        pass

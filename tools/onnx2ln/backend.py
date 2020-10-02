# This file is based on https://github.com/onnx/onnx-tensorflow

import json
import warnings

import onnx
from onnx import TensorProto
from onnx import numpy_helper
from onnx import shape_inference
# from onnx_tf.common import data_type
from pb_wrapper import OnnxNode
from pb_wrapper import OnnxGraph
from node_converter import new_opname
from node_converter import onnx_node_to_ln_op

TENSOR_TYPE_TO_TL_TYPE = {
    int(TensorProto.FLOAT): 'TL_FLOAT',
    int(TensorProto.UINT8): 'TL_UINT8',
    int(TensorProto.INT8): 'TL_INT8',
    int(TensorProto.UINT16): 'TL_UINT16',
    int(TensorProto.INT16): 'TL_INT16',
    int(TensorProto.INT32): 'TL_INT32',
    int(TensorProto.INT64): 'TL_INT64',
    int(TensorProto.BOOL): 'TL_BOOL',
    int(TensorProto.FLOAT16): 'TL_DTYPE_INVALID',
    int(TensorProto.DOUBLE): 'TL_DOUBLE',
    int(TensorProto.COMPLEX64): 'TL_DTYPE_INVALID',
    int(TensorProto.COMPLEX128): 'TL_DTYPE_INVALID',
    int(TensorProto.UINT32): 'TL_UINT32',
    int(TensorProto.UINT64): 'TL_UINT64',
    int(TensorProto.STRING): 'TL_DTYPE_INVALID'
}

TENSOR_TYPE_NAME = {
    int(TensorProto.FLOAT): 'FLOAT',
    int(TensorProto.UINT8): 'UINT8',
    int(TensorProto.INT8): 'INT8',
    int(TensorProto.UINT16): 'UINT16',
    int(TensorProto.INT16): 'INT16',
    int(TensorProto.INT32): 'INT32',
    int(TensorProto.INT64): 'INT64',
    int(TensorProto.BOOL): 'TL_BOOL',
    int(TensorProto.FLOAT16): 'FLOAT16',
    int(TensorProto.DOUBLE): 'DOUBLE',
    int(TensorProto.COMPLEX64): 'COMPLEX64',
    int(TensorProto.COMPLEX128): 'COMPLEX128',
    int(TensorProto.UINT32): 'UINT32',
    int(TensorProto.UINT64): 'UINT64',
    int(TensorProto.STRING): 'STRING'
}

def dtype_onnx2tl(name, onnx_dtype):
    tl_dtype = TENSOR_TYPE_TO_TL_TYPE[onnx_dtype]
    if tl_dtype == 'TL_DTYPE_INVALID':
        warnings.warn("Can't convert onnx dtype {} of tensor {} to TL_DTYPE. Return TL_DTYPE_INVALID.".format(TENSOR_TYPE_NAME[onnx_dtype], name))
    return tl_dtype

def get_model(onnx_graph):
    # onnx_graph = shape_inference.infer_shapes(onnx_graph)
    # onnx.checker.check_model(onnx_graph)
    if onnx_graph.initializer:
        input_tensors = onnx_initializer_to_data_tensors(
            onnx_graph.initializer);
        initialized = {init.name for init in onnx_graph.initializer}
    else:
        input_tensors = []
        initialized = set()

    # creating empty tensors for currently unknown inputs
    for value_info in onnx_graph.input:
        if value_info.name in initialized:
            continue
        shape = list(
            d.dim_value if (d.dim_value > 0 and d.dim_param == "") else None
            for d in value_info.type.tensor_type.shape.dim)
        tensor = {'name': value_info.name,
                  'dtype': dtype_onnx2tl(value_info.name, value_info.type.tensor_type.elem_type),
                  'dims': shape,
                  'data': None}
        input_tensors.append((value_info.name, tensor))

    input_dict = dict(input_tensors)
    tensor_dict = dict(input_tensors)
    model = {'ops': []}

    for tensor in input_tensors:
        model['ops'].append(new_create_op(tensor[1]))

    for node in onnx_graph.node:
        onnx_node = OnnxNode(node)
        ops = onnx_node_to_ln_op(onnx_node, tensor_dict)
        for op in ops:
            model['ops'].append(op)

    return model

def onnx_initializer_to_data_tensors(initializer):
    """ Convert ONNX graph initializer to input tensor items.

    :param initializer: ONNX graph initializer, list of TensorProto.
    :return: List of input tensor items.
    """

    def tensor2list(onnx_tensor):
        # Use the onnx.numpy_helper because the data may be raw
        return numpy_helper.to_array(onnx_tensor).flatten().tolist()
    tensors = [(init.name, {'name': init.name,
                            'dtype': dtype_onnx2tl(init.name, init.data_type),
                            'dims': list(d for d in init.dims),
                            'data': tensor2list(init)})
               for init in initializer]
    return tensors

def new_create_op(tensor):
    data = [0] if tensor['data'] is None else tensor['data']
    op = {'name': new_opname("create"),
          'optype': 'create',
          'tensors_in': [],
          'tensors_out': [{'arg_name': 'dst', 'name': tensor['name']}],
          'params': [{'arg_name': 'dtype', 'value': tensor['dtype']},
                     {'arg_name': 'dims', 'value': tensor['dims']},
                     {'arg_name': 'data', 'value': data},
                     {'arg_name': 'ran', 'value': [0, 0]},
                     {'arg_name': 'from_file', 'value': False}]}
    return op

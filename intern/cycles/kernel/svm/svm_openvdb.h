/*
 * Copyright 2015 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

CCL_NAMESPACE_BEGIN

#ifdef __OPENVDB__

ccl_device void svm_node_openvdb(KernelGlobals *kg, ShaderData *sd, float *stack, uint4 node, int *offset)
{
	uint slot = node.y;
	uint type, out_offset, sampling, co_offset;
	decode_node_uchar4(node.z, &type, &co_offset, &out_offset, &sampling);

	float3 co = stack_load_float3(stack, co_offset);

	Transform tfm;
	tfm.x = read_node_float(kg, offset);
	tfm.y = read_node_float(kg, offset);
	tfm.z = read_node_float(kg, offset);
	tfm.w = read_node_float(kg, offset);
	co = transform_point(&tfm, co);

#ifdef __KERNEL_GPU__
	float3 out = make_float3(0.0f, 0.0f, 0.0f);
#else
	if(type == NODE_VDB_FLOAT) {
		float out = 0.0f;
		if(sampling == OPENVDB_SAMPLE_POINT) {
			out = kg->vdb_float_samplers_p[slot]->wsSample(openvdb::Vec3d(co.x, co.y, co.z));
		}
		else {
			out = kg->vdb_float_samplers_b[slot]->wsSample(openvdb::Vec3d(co.x, co.y, co.z));
		}

		if(stack_valid(out_offset)) {
			stack_store_float(stack, out_offset, out);
		}
	}
	else if(type == NODE_VDB_VEC3S) {
		openvdb::Vec3s r = openvdb::Vec3s(0.0f);
		if(sampling == OPENVDB_SAMPLE_POINT) {
			r = kg->vdb_vec3s_samplers_p[slot]->wsSample(openvdb::Vec3d(co.x, co.y, co.z));
		}
		else {
			r = kg->vdb_vec3s_samplers_b[slot]->wsSample(openvdb::Vec3d(co.x, co.y, co.z));
		}

		float3 out = make_float3(r.x(), r.y(), r.z());

		if(stack_valid(out_offset)) {
			stack_store_float3(stack, out_offset, out);
		}
	}
#endif
}

#endif

CCL_NAMESPACE_END


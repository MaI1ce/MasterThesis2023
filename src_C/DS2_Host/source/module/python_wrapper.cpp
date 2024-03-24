#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/operators.h>

#include "../ds2/ds2_host_class.h"

namespace py = pybind11;
using namespace pybind11::literals;


PYBIND11_MODULE(ds2, handle) {

	handle.doc() = "Module name: DS2_H \n \
					Author:	Vladyslav Shapoval \n \
					Master Thesis 2024 ";

	py::register_exception<DS2Exception>(handle, "DS2Exception");

	py::class_<ds2_host>(handle, "ds2_host")
		.def(py::init())
		.def("get_parties_num", &ds2_host::get_party_num)
		.def("set_pi_commit", &ds2_host::set_pi_commit)
		.def("set_ti_commit", &ds2_host::set_ti_commit)
		.def("set_fi_commit", &ds2_host::set_fi_commit)
		.def("set_pi_val", &ds2_host::set_pi_val)
		.def("set_ti_val", &ds2_host::set_ti_val)
		.def("set_ri_val", &ds2_host::set_ri_val)
		.def("set_zi_1_val", &ds2_host::set_zi_1_val)
		.def("set_zi_2_val", &ds2_host::set_zi_2_val)
		.def("reset", &ds2_host::reset)
		.def("is_flag_ready", &ds2_host::is_flag_ready);
}
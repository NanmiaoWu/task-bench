//  Copyright (c)      2021 Nanmiao Wu
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "core.h"

#include "hpx/hpx.hpp"
#include "hpx/local/chrono.hpp"

int hpx_main(int argc, char *argv[]) 
{ 

  // get number of threads and this thread
  std::uint32_t num_threads = hpx::get_num_localities(hpx::launch::sync);
  std::uint32_t this_thread = hpx::get_locality_id();

  App app(argc, argv);
  if (this_thread == 0) app.display();

  std::vector<std::vector<char> > scratch;

  for (auto graph : app.graphs) {
    long first_point = this_thread * graph.max_width / num_threads;
    long last_point = (this_thread + 1) * graph.max_width / num_threads - 1;
    long n_points = last_point - first_point + 1;

    size_t scratch_bytes = graph.scratch_bytes_per_task;
    scratch.emplace_back(scratch_bytes * n_points);
    TaskGraph::prepare_scratch(scratch.back().data(), scratch.back().size());
  }

  
  for (int iter = 0; iter < 2; ++iter) {
    hpx::chrono::high_resolution_timer timer;

    for (auto graph : app.graphs) {
      long first_point = this_thread * graph.max_width / n_threads;
      long last_point = (this_thread + 1) * graph.max_width / n_threads - 1;
      long n_points = last_point - first_point + 1;

      size_t scratch_bytes = graph.scratch_bytes_per_task;
      char *scratch_ptr = scratch[graph.graph_index].data();

      std::vector<int> thread_by_point(graph.max_width);
      for (int r = 0; r < num_threads; ++r) {
        long r_first_point = r * graph.max_width / num_threads;
        long r_last_point = (r + 1) * graph.max_width / num_threads - 1;
        for (long p = r_first_point; p <= r_last_point; ++p) {
          thread_by_point[p] = r;
        }
      }

      long max_deps = 0;
      for (long dset = 0; dset < graph.max_dependence_sets(); ++dset) {
        for (long point = first_point; point <= last_point; ++point) {
          long deps = 0;
          for (auto interval : graph.dependencies(dset, point)) {
            deps += interval.second - interval.first + 1;
          }
          max_deps = std::max(max_deps, deps);
        }
      }

      // Create input and output buffers.
      std::vector<std::vector<std::vector<char> > > inputs(n_points);
      std::vector<std::vector<const char *> > input_ptr(n_points);
      std::vector<std::vector<size_t> > input_bytes(n_points);
      std::vector<long> n_inputs(n_points);
      std::vector<std::vector<char> > outputs(n_points);
      for (long point = first_point; point <= last_point; ++point) {
        long point_index = point - first_point;

        auto &point_inputs = inputs[point_index];
        auto &point_input_ptr = input_ptr[point_index];
        auto &point_input_bytes = input_bytes[point_index];

        point_inputs.resize(max_deps);
        point_input_ptr.resize(max_deps);
        point_input_bytes.resize(max_deps);

        for (long dep = 0; dep < max_deps; ++dep) {
          point_inputs[dep].resize(graph.output_bytes_per_task);
          point_input_ptr[dep] = point_inputs[dep].data();
          point_input_bytes[dep] = point_inputs[dep].size();
        }

        auto &point_outputs = outputs[point_index];
        point_outputs.resize(graph.output_bytes_per_task);
      }

      // Cache dependencies.
      std::vector<std::vector<std::vector<std::pair<long, long> > > > dependencies(graph.max_dependence_sets());
      std::vector<std::vector<std::vector<std::pair<long, long> > > > reverse_dependencies(graph.max_dependence_sets());
      for (long dset = 0; dset < graph.max_dependence_sets(); ++dset) {
        dependencies[dset].resize(n_points);
        reverse_dependencies[dset].resize(n_points);

        for (long point = first_point; point <= last_point; ++point) {
          long point_index = point - first_point;

          dependencies[dset][point_index] = graph.dependencies(dset, point);
          reverse_dependencies[dset][point_index] = graph.reverse_dependencies(dset, point);
        }
      }

      // begins 
      std::vector<std::vector<hpx::lcos::future<void>>> futures;

      for (long timestep = 0; timestep < graph.timesteps; ++timestep) {
        long offset = graph.offset_at_timestep(timestep);
        long width = graph.width_at_timestep(timestep);

        long last_offset = graph.offset_at_timestep(timestep-1);
        long last_width = graph.width_at_timestep(timestep-1);

        long dset = graph.dependence_set_at_timestep(timestep);
        auto &deps = dependencies[dset];
        auto &rev_deps = reverse_dependencies[dset];

        std::vector<hpx::lcos::future<void>> future_vec;

        for (long point = first_point; point <= last_point; ++point) {
          long point_index = point - first_point;

          auto &point_inputs = inputs[point_index];
          auto &point_n_inputs = n_inputs[point_index];
          auto &point_output = outputs[point_index];

          auto &point_deps = deps[point_index];
          auto &point_rev_deps = rev_deps[point_index];

          if (point >= offset && point < offset + width) {
              
          }


        }

        futures.push_back(future_vec);
      
      }


    
    }

    
    
    double elapsed = timer.elapsed(); 
  }

  if (this_thread == 0) {
    app.report_timing(elapsed);
  }

  return hpx::finalize();;
}

int main(int argc, char *argv[]) 
{ 
  return hpx::init(argc, argv);
}


#include "dyn_msd.h"
#include "defs.h"
#include "pbc.h"
#include "eval_isoconf.h"
#include "eval_struct.h"

void eval_msd(){

    int flag = msd_flag;

    double * save_bb_traj = new double [NS*NT*NI*N];

    // loop over time
    for (int t=1; t<NT; t++) {
        std::cout << "EVAL MD " << t << std::endl; 

        // first calculate bonds (by just iterating over one trajectory)
        if (dyn_ranges[flag][0] > -10000.0) {
            dyn_ranges_time[flag][2*t] = dyn_ranges[flag][0];
            dyn_ranges_time[flag][2*t+1] = dyn_ranges[flag][1];
        } else { // determine binning from maximal and minimal values (just iterate over one simulation)
            double dyn_loc[N*NS] = {0};
            for (int s=0; s<NS;s++) { // loop over structures
                for (int i=0; i<N;i++) {
                    double dr = 0, dx;
                    for (int d=0; d<NDim;d++) {
                        dx = xyz_data[i+s*N][d] - xyz_data[i+s*N][d+t*NDim];
                        apply_pbc(dx);
                        dr += dx*dx;
                    }
                    dyn_loc[s*N+i] = dr;
                }
            }
            calc_bonds(dyn_loc,&dyn_ranges_time[flag][2*t]);
        }
        if (dyn_ranges[flag+1][0] > -10000.0) {
            dyn_ranges_time[flag+1][2*t] = dyn_ranges[flag+1][0];
            dyn_ranges_time[flag+1][2*t+1] = dyn_ranges[flag+1][1];
        } else { // determine binning from maximal and minimal values (just iterate over one simulation)
            double dyn_loc[N*NS] = {0};
            for (int s=0; s<NS;s++) { // loop over structures
                for (int i=0; i<N;i++) {
                    double dr = 0, dx;
                    for (int d=0; d<NDim;d++) {
                        dx = xyz_data[i+s*N][d] - xyz_data[i+s*N][d+t*NDim];
                        apply_pbc(dx);
                        dr += dx*dx;
                    }
                    dyn_loc[s*N+i] = log(dr);
                }
            }
            calc_bonds(dyn_loc,&dyn_ranges_time[flag+1][2*t]);
        }

        std::cout << "EVAL MD 2 " << t << std::endl; 

        // the main evaluation for the isoconfigurational ensemble (mean displacement)
        reset_dyn(t);
        for (int s=0; s<NS;s++) { // loop over structures
            for (int i=0; i<N;i++) {
                for (int j=0; j<NI;j++) {
                    //std::cout << s << " " << i << " " << j  << std::endl; 
                    double dr = 0, dx;
                    for (int d=0; d<NDim;d++) {
                        dx = xyz_data[i+s*N][d+NDim*NT*j] - xyz_data[i+s*N][d+t*NDim+NDim*NT*j];
                        apply_pbc(dx);
                        dr += dx*dx;
                    }
                    dr = sqrt(dr);
                    add_histogram_avg(s,i,j,&dyn_ranges_time[flag][2*t],dr);
                    save_bb_traj[s*NT*NI*N+t*N*NI+j*N+i] =dr;
                }
            }
        }
        eval_isoconf(t, flag);

        // the main evaluation for the isoconfigurational ensemble (log of mean displacement)
        std::cout << "EVAL LOG(MD) " << t << std::endl; 
        reset_dyn(t);
        for (int s=0; s<NS;s++) { // loop over structures
            for (int i=0; i<N;i++) {
                for (int j=0; j<NI;j++) {
                    double dr = 0, dx;
                    for (int d=0; d<NDim;d++) {
                        dx = xyz_data[i+s*N][d+NDim*NT*j] - xyz_data[i+s*N][d+t*NDim+NDim*NT*j];
                        apply_pbc(dx);
                        dr += dx*dx;
                    }
                    dr = sqrt(dr);
                    //std::cout << dr << std::endl;
                    if (dr < 10e-6) dr = 10e-6;
                    dr = log10(dr);
                    //std::cout << dr << std::endl;
                    //if (save_pat[2*NS*NI*N+s*N*NI+j*N+i] < 0.5) {
                        add_histogram_avg(s,i,j,&dyn_ranges_time[flag+1][2*t],dr);
                    //}
                }
            }
        }
        //std::cout << "FINISHED LOG(MD) " << t << std::endl; 
        std::cout << "EVAL MD: ISOCONF " << t << std::endl; 
        eval_isoconf(t, flag+1);

    }


    // just S and G
    if (boxL > 100.0) {
        double tmp[N*NS];
        for (int s=0; s<NS; s++) {
            for (int i=0; i<N; i++) {
                tmp[i] = 1.0;
            }
        }
        std::string tmps;

        if (bb_flag==-1) {
            std::string tmps = "BASE";
            int first =1;
            eval_struct(tmp,tmps,first);
        }

        // calc S4, G4
        for (int t=25; t<35; t++) {
            int first = 0;

            for (int i=0; i<N*NS; i++) {
                tmp[i] = ( tanh(  (dyn_avg_save[i][flag*(NT+1)+t]-overlap_cut)*20.0  ) + 1.0)/2.0;
            }
            tmps = "MD"+std::to_string(t);
            
            eval_struct(tmp,tmps,first);
            for (int j=0; j<NI;j++) {
                for (int s=0; s<NS; s++) {
                    for (int i=0; i<N; i++) {
                        tmp[i] = ( tanh(  (save_bb_traj[s*NT*NI*N+t*N*NI+j*N+i]-overlap_cut)*20.0  ) + 1.0)/2.0;
                    }
                }
                tmps = "MD"+std::to_string(t)+"Traj"+std::to_string(j);
                eval_struct(tmp,tmps,first);
            }

        }
    }

    // write results
    print_traj(save_bb_traj, flag);

    // write results
    print_isoconf(flag);
    print_isoconf(flag+1);

}
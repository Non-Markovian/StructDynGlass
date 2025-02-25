#include "struct_soft_modes.h"
#include "struct_base.h"
#include "defs.h"
#include "pbc.h"
#include <bits/stdc++.h> 
#include <Eigen/Dense>

using namespace Eigen;

void eval_struct_soft_modes(){

    std::cout << "EVAL STRUCT SOFT MODES " << std::endl; 

    for (int s=0; s<NS;s++) {
        std::cout << "EVAL STRUCT SOFT MODES: STURCTURE " << s << std::endl; 

        // calculate hessian
        double result_loc[NDim*NDim], dx[NDim];
        for (int i=0; i<N;i++) { // loop over particles
            for (int j=0; j<N;j++) { // loop over particles
                calc_2Depot(i+s*N,j+s*N,0,0,result_loc,dx,hessian[i*N+j]);
            }
        }

        // test harmonic
        /*if (s==0) {
            int i=1;
            int j=2;
            int d1=0;
            int d2=1;
            double shift = 0.00;
            double delta = 0.000001;
            xyz_inherent_data[i+s*N][d1] -= shift;
            xyz_inherent_data[i+s*N][d1] -= delta;
            double epot_im1 = calc_epot_tot(s);
            xyz_inherent_data[i+s*N][d1] += 2.0*delta;
            double epot_ip1 = calc_epot_tot(s);
            xyz_inherent_data[i+s*N][d1] -= delta;
            xyz_inherent_data[i+s*N][d1] += shift;

            std::cout << epot_ip1 << " " << epot_im1 << " " <<  0.5/delta*(epot_ip1-epot_im1) << std::endl;

            // test hessian
            xyz_inherent_data[i+s*N][d1] -= delta;
            xyz_inherent_data[j+s*N][d2] -= delta;
            double epot_im1_jm1 = calc_epot_tot(s);
            xyz_inherent_data[i+s*N][d1] += 2.0*delta;
            double epot_ip1_jm1 = calc_epot_tot(s);
            xyz_inherent_data[j+s*N][d2] += 2.0*delta;
            double epot_ip1_jp1 = calc_epot_tot(s);
            xyz_inherent_data[i+s*N][d1] -= 2.0*delta;
            double epot_im1_jp1 = calc_epot_tot(s);
            xyz_inherent_data[i+s*N][d1] += delta;
            xyz_inherent_data[j+s*N][d2] -= delta;
            std::cout << hessian[s*N*N+i*N+j][d1*NDim+d2] << " " << 0.25/delta/delta*(epot_ip1_jp1-epot_im1_jp1-epot_ip1_jm1+epot_im1_jm1) << std::endl;
        }*/

        if (modeSM==0) { // EigenDecomposition needs to be calculated

            MatrixXd Eigen_hessian;

            Eigen_hessian = MatrixXd::Zero(N*NDim,N*NDim);
            for (int i=0; i<N;i++) { // loop over particles
                for (int d1=0; d1<NDim;d1++) {
                    for (int j=0; j<N;j++) { // loop over particles
                        for (int d2=0; d2<NDim;d2++) {
                            Eigen_hessian(i*NDim+d1,j*NDim+d2) = hessian[i*N+j][d1*NDim+d2];
                        }
                    }
                }
            }

            //std::cout << Eigen_hessian << std::endl;

            
            // the actual eigen decomposition
            std::cout << "Eigen Decomposition " << s << std::endl; 
            EigenSolver<MatrixXd> Eigen_decomposition(Eigen_hessian);
            //std::cout << "The eigenvalues of A are:" << std::endl << Eigen_decomposition.eigenvalues() << std::endl;
            //VectorXcd vT = Eigen_decomposition.eigenvectors().col(5);
            //std::cout << "The corresponding eigenvector is:" << std::endl << vT << std::endl; 

            VectorXcd ev = Eigen_decomposition.eigenvalues();

            // calculate ranks
            double save_struct[NDim*N]; 
            for(int i = 0; i < NDim*N; i++) {
                if ( ev(i).real() < -0.00001) {
                    std::cout << "Negative Eigenvalue! " << ev(i).real() <<  std::endl;
                    std::cout << "Set to 100 such that it does not contribute! "  <<  std::endl;
                    ev(i).real(100.0);
                }
                save_struct[i] = ev(i).real(); 
                std::cout << ev(i).real() << std::endl;
            }
            std::sort(save_struct, save_struct+NDim*N); // sorting the array 
            std::map<double, int> rank_struct; 
            for (int i = 0; i < NDim*N; i++) { 
                rank_struct[save_struct[i]] = i; 
            }
            MatrixXcd evec = Eigen_decomposition.eigenvectors();
            for (int k=0; k<N*NDim;k++) { 
                int ind = rank_struct[ev[k].real()] -NDim;
                // -NDim because NDim evalues are zero due to translational invariance
                if (ind>=0) {
                    hessian_evalues[s*N*NDim+ind] = sqrt(ev[k].real());
                    VectorXcd vT = evec.col(k);
                    for (int i=0; i<N*NDim;i++) { 
                        hessian_evectors[ind][i] = vT[i].real();
                    }
                }

                //std::cout << ev[k].real()  <<  std::endl;
            }
            // low level sanity checks
            /*for (int k=0; k<N*NDim-NDim;k++) { 
                std::cout << k << " " << hessian_evalues[s*N*NDim+k] << std::endl;
            }*/
            // check that evectors are indeed eigenvectors
            /*int k=3;
            VectorXcd vT = evec.col(k);
            for (int i=0; i<N*NDim;i++) { 
                //double resultMult = 0.0;
                //for (int j=0; j<N*NDim;j++) { 
                //    resultMult += Eigen_hessian(i,j)*vT[j].real();
                //}
                //std::cout << resultMult << " " << vT[i].real()*ev[k].real() << std::endl;

                double resultMult = 0.0;
                for (int j=0; j<N*NDim;j++) { 
                    resultMult += Eigen_hessian(i,j)*hessian_evectors[s*N*NDim+k][j];
                }
                std::cout << resultMult << " " << hessian_evectors[s*N*NDim+k][i]*hessian_evalues[s*N*NDim+k]*hessian_evalues[s*N*NDim+k] << std::endl;
            
            }*/

            // Write eigen decomposition
            QString path = QString::fromStdString(lammpsIn);
            path.append(QString("/Cnf-%1/").arg(CnfStart+s*CnfStep));
            path.append("eigenDecomposition.dat");
            QFile outfileEigen(path);  
            outfileEigen.open(QIODevice::WriteOnly | QIODevice::Text);
            QTextStream outEigen(&outfileEigen);
            for (int k=0; k<N*NDim-NDim;k++) { 
                outEigen <<  hessian_evalues[s*N*NDim+k] << " ";
                for (int i=0; i<N*NDim;i++) { 
                    outEigen << hessian_evectors[k][i] << " ";
                }
                outEigen << "\n";
            }
            outfileEigen.close();
            
        } else { // Eigen Decomposition can be read

            QString path = QString::fromStdString(lammpsIn);
            path.append(QString("/Cnf-%1/").arg(CnfStart+s*CnfStep));
            path.append("eigenDecomposition.dat");
            QFile outfileEigen(path);  
            outfileEigen.open(QIODevice::ReadOnly | QIODevice::Text);
            QTextStream outEigen(&outfileEigen);
            for (int k=0; k<N*NDim-NDim;k++) { 
                outEigen >>  hessian_evalues[s*N*NDim+k];
                if ( !(hessian_evalues[s*N*NDim+k] > 0.1) ) {
                    std::cout << "Small Eigenvalue! " << hessian_evalues[s*N*NDim+k] <<  std::endl;
                    std::cout << "Set to 10000 such that it does not contribute! "  <<  std::endl;
                    hessian_evalues[s*N*NDim+k] = 10000.0;
                }
                for (int i=0; i<N*NDim;i++) { 
                    outEigen >> hessian_evectors[k][i];
                }
            }
            outfileEigen.close();
            
        }

        // calculate participation ratio
        for (int k=0; k<N*NDim-NDim;k++) {
            double Pw_mean = 0.0;
            double Pw = 0.0;
            for (int i=0; i<N;i++) {
                double resloc=0.0;
                for (int di=0; di<NDim;di++) resloc+=hessian_evectors[k][i*NDim+di]*hessian_evectors[k][i*NDim+di];
                Pw_mean += resloc;
                Pw += resloc*resloc;
            }
            //std::cout << k << " " << hessian_evalues[s*N*NDim+k] << " " << Pw_mean << " " << 1.0/(N*Pw) << std::endl;
            participation_ratio[s*N*NDim+k] = 1.0/(N*Pw);
        }
        

        // calculate participation ratio for lowest frequency modes
        for (int i=0; i<N;i++) {
            struct_local[NCG*(struct_soft_modes_flag)][i+s*N] = 0.0;
            for (int k=0; k<NLOW;k++) {
                if(participation_ratio[s*N*NDim+k] < Pcut ) {
                    for (int di=0; di<NDim;di++) struct_local[NCG*(struct_soft_modes_flag)][i+s*N] += hessian_evectors[k][i*NDim+di]*hessian_evectors[k][i*NDim+di];
                    //std::cout << k << " " << participation_ratio[s*N*NDim+k] << std::endl;
                }
            }   
        }

        // calculate vibrality
        for (int i=0; i<N;i++) {
            struct_local[NCG*(struct_soft_modes_flag+1)][i+s*N] = 0.0;
            for (int k=0; k<N*NDim-NDim;k++) {
                double result_loc = 0.0;
                for (int di=0; di<NDim;di++) result_loc += hessian_evectors[k][i*NDim+di]*hessian_evectors[k][i*NDim+di];
                struct_local[NCG*(struct_soft_modes_flag+1)][i+s*N] += result_loc/(hessian_evalues[s*N*NDim+k]*hessian_evalues[s*N*NDim+k]);
            }   
            //std::cout << struct_local[NCG*(struct_soft_modes_flag+1)][i+s*N] << std::endl;
        }
        
    }

    // coarse-grain quantities
    double mean_den[NCG];
    double mean_sm[NCG];
    double mean_vibrality[NCG];
    for (int s=0; s<NS;s++) { // loop over structures
        for (int i=0; i<N;i++) { // loop over particles
            for (int c=0; c<NCG; c++) {
                mean_den[c] = 0.0;
                mean_sm[c] = 0.0;
                mean_vibrality[c]=0.0;
            }
            for (int j=0; j<N;j++) { // loop over particle pairs
                double dr = 0.0, dx;
                for (int d=0; d<NDim;d++) {
                    dx = xyz_data[i+s*N][d] - xyz_data[j+s*N][d];
                    apply_pbc(dx);
                    dr += dx*dx;
                }
                dr = sqrt(dr);
                for (int c=0; c<NCG; c++) {
                    double L = c;
                    if (L < 0.1) L = 0.1;
                    double w = exp(-dr/L);
                    mean_den[c] += w;
                    mean_sm[c] += w*struct_local[NCG*(struct_soft_modes_flag)][j+s*N];
                    mean_vibrality[c] += w*struct_local[NCG*(struct_soft_modes_flag+1)][j+s*N];
                }
            }
            for (int c=0; c<NCG; c++) {
                double L = c;
                if (L < 0.1) L = 0.1;
                struct_local[NCG*(struct_soft_modes_flag)+c][i+s*N] = mean_sm[c]/mean_den[c];
                struct_local[NCG*(struct_soft_modes_flag+1)+c][i+s*N] = mean_vibrality[c]/mean_den[c];
            }

        }
    }

    std::cout << "EVAL STRUCT SOFT MODES: FINISHED " << std::endl; 
}





// help functions

// calc hessian matrix
void calc_2Depot(int i, int j, int t, int iso, double * result_loc, double * dx, double * result) {

    // first reset result
    for (int d1=0; d1<NDim;d1++) {
        for (int d2=0; d2<NDim;d2++) {
            result[d1*NDim+d2] = 0.0;
        }
    }

    if (i==j) { // need to sum over all neighbors
        // calculate types
        int iType = type_data[i];
        int s = (i - i % N)/N;
        //std::cout << "s " << s << std::endl;
        for (int k=0; k<N;k++) { // loop over particle pairs
            double dr = 0.0;
            for (int d=0; d<NDim;d++) {
                dx[d] = xyz_inherent_data[i][d+t*NDim+NDim*NT*iso] - xyz_inherent_data[k+s*N][d+t*NDim+NDim*NT*iso];
                apply_pbc(dx[d]);
                dr += dx[d]*dx[d];
            }
            double sigma = determine_sigma(i, k+s*N);
            double sigma2= sigma*sigma;

            if(model=="KA2" || model=="KA2-2D" || model=="KA") {
                if (dr < RC2LJ*sigma2 && i!=k+s*N ) {
                    //std::cout << i << " " << k << std::endl;
                    int kType = type_data[k+s*N];
                    double epsilon = determine_epsilon(iType, kType);
                    double rij2i =  1.0/dr;
                    double rij6i = sigma2*sigma2*sigma2*rij2i*rij2i*rij2i;
                    double rij12i = rij6i*rij6i;
                    double c2 = C2LJ / (sigma2);
                    double c4 = C4LJ / (sigma2*sigma2);
                    double E = 4.0 * epsilon * (8.0*c4 + 168.0*rij12i*rij2i*rij2i - 48.0*rij6i*rij2i*rij2i);
                    double F = 4.0 * epsilon * (2.0*c2 + 4.0*c4*dr - 12.0*rij12i*rij2i + 6.0*rij6i*rij2i);
                    for (int d1=0; d1<NDim;d1++) {
                        for (int d2=0; d2<=d1;d2++) {
                            if (d1==d2) result[d1*NDim+d2] += E*dx[d1]*dx[d1] + F;
                            else result[d1*NDim+d2] += E*dx[d1]*dx[d2];
                        }
                    }
                }
            }
            if(model=="POLY") {
                if (dr < RC2POLY*sigma2 && i!=k+s*N ) {
                    //std::cout << i << " " << k << std::endl;
                    int kType = type_data[k+s*N];
                    double epsilon = determine_epsilon(iType, kType);
                    double rij2i =  1.0/dr;
                    double sigma2= sigma*sigma;
                    double rij6i = sigma2*sigma2*sigma2*rij2i*rij2i*rij2i;
                    double rij12i = rij6i*rij6i;
                    double c2 = C2POLY / (sigma2);
                    double c4 = C4POLY / (sigma2*sigma2);
                    double E = 4.0 * epsilon * (8.0*c4 + 168.0*rij12i*rij2i*rij2i);
                    double F = 4.0 * epsilon * (2.0*c2 + 4.0*c4*dr - 12.0*rij12i*rij2i);
                    for (int d1=0; d1<NDim;d1++) {
                        for (int d2=0; d2<=d1;d2++) {
                            if (d1==d2) result[d1*NDim+d2] += E*dx[d1]*dx[d1] + F;
                            else result[d1*NDim+d2] += E*dx[d1]*dx[d2];
                        }
                    }
                }
            }
        }

        for (int d1=0; d1<NDim-1;d1++) {
            for (int d2=d1+1; d2<NDim;d2++) {
                result[d1*NDim+d2] = result[d2*NDim+d1];
            }
        }

    } else {
        // calculate types
        int iType = type_data[i];
        int jType = type_data[j];

        // calculate interaction parameters
        double sigma = determine_sigma(i, j);
        double epsilon = determine_epsilon(iType, jType);

        double dr = 0.0;
        for (int d=0; d<NDim;d++) {
            dx[d] = xyz_inherent_data[i][d+t*NDim+NDim*NT*iso] - xyz_inherent_data[j][d+t*NDim+NDim*NT*iso];
            apply_pbc(dx[d]);
            dr += dx[d]*dx[d];
        }
        double sigma2= sigma*sigma;

        if(model=="KA2" || model=="KA2-2D" || model=="KA") {
            if (dr < RC2LJ*sigma2 ) {
                double rij2i =  1.0/dr;
                double rij6i = sigma2*sigma2*sigma2*rij2i*rij2i*rij2i;
                double rij12i = rij6i*rij6i;
                double c2 = C2LJ / (sigma2);
                double c4 = C4LJ / (sigma2*sigma2);
                double E = -4.0 * epsilon * (8.0*c4 + 168.0*rij12i*rij2i*rij2i - 48.0*rij6i*rij2i*rij2i);
                double F = -4.0 * epsilon * (2.0*c2 + 4.0*c4*dr - 12.0*rij12i*rij2i + 6.0*rij6i*rij2i);
                for (int d1=0; d1<NDim;d1++) {
                    for (int d2=0; d2<NDim;d2++) {
                        if (d1==d2) result[d1*NDim+d2] = E*dx[d1]*dx[d1] + F;
                        else {
                            if (d1 > d2) result[d1*NDim+d2] = result[d2*NDim+d1];
                            else result[d1*NDim+d2] = E*dx[d1]*dx[d2];
                        }
                    }
                }
            } 
        }
        if(model=="POLY") {
            if (dr < RC2POLY*sigma2) {
                double result_loc[NDim*NDim];
                double rij2i =  1.0/dr;
                double sigma2= sigma*sigma;
                double rij6i = sigma2*sigma2*sigma2*rij2i*rij2i*rij2i;
                double rij12i = rij6i*rij6i;
                double c2 = C2POLY / (sigma2);
                double c4 = C4POLY / (sigma2*sigma2);
                double E = 4.0 * epsilon * (8.0*c4 + 168.0*rij12i*rij2i*rij2i);
                double F = 4.0 * epsilon * (2.0*c2 + 4.0*c4*dr - 12.0*rij12i*rij2i);
                for (int d1=0; d1<NDim;d1++) {
                    for (int d2=0; d2<NDim;d2++) {
                        if (d1==d2) result[d1*NDim+d2] = E*dx[d1]*dx[d1] + F;
                        else {
                            if (d1 > d2) result[d1*NDim+d2] = result[d2*NDim+d1];
                            else result[d1*NDim+d2] = E*dx[d1]*dx[d2];
                        }
                    }
                }
            } 
        }
    }

    /*if( (i==5 && j==6) ||(i==5 && j==5))
        for (int d1=0; d1<NDim;d1++) {
            for (int d2=0; d2<NDim;d2++) {
                std::cout << i << " " << j << " " << result[d1*NDim+d2] << std::endl;
            }
        }*/
}

void calc_3Depot(int i, int j, int k, double * result) {
    /*double sigma = determine_sigma(iType, jType);
    double epsilon = determine_epsilon(iType, jType);

    if (dist2 < RC2 ) {
        double rij2 = dist2/(sigma*sigma);
        double rij4 = rij2*rij2;
        double rij6 = 1.0/(rij4*rij2);
        double rij12 = rij6*rij6;
         4.0*epsilon*(C0+C2*rij2+C4*rij4 - rij6 + rij12 ) ;
    } */
}

double calc_epot_tot(int s){
    double epot=0.0;
    for (int i=0; i<N;i++) { // loop over particles
        int iType = type_data[i+s*N];

        for (int j=0; j<N;j++) { // loop over particle pairs
            int jType = type_data[j+s*N];
            double dr = 0.0, dx;
            for (int d=0; d<NDim;d++) {
                dx = xyz_inherent_data[i+s*N][d] - xyz_inherent_data[j+s*N][d];
                apply_pbc(dx);
                dr += dx*dx;
            }

            // calculate epot
            if (i!=j) epot += 0.5*calc_epot(i+s*N, j+s*N, dr);
        }
    }
    return epot;
}
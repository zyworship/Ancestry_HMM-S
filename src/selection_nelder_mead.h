#ifndef __SELECTION_NELDER_MEAD_H
#define __SELECTION_NELDER_MEAD_H


// sorts vertex. can probably be optimized
void selection_sort_vertex( vector<selection> &v ) {
    sort(v.begin(), v.end());
    reverse(v.begin(), v.end());
}

// generates random double between fmin and fmax
double double_rand(double fmin, double fmax) {
    double f = (double)rand() / RAND_MAX;
    return fmin + f * (fmax - fmin);
}

// generates transition matrix.
void selection_transition_matrix(map<int,vector<mat> > &transition_matrix , vector<vector< map< vector<transition_information>, double > > > &transition_info, vector<double> &recombination_rate, vector<int> &positions, double &number_chromosomes, vector<mat> &transition_rates ) {
    
    /// check if we already computed this for this sample ploidy
    if ( transition_matrix.find( number_chromosomes ) != transition_matrix.end() ) {
        return ;
    }
    
    /// else, have to create entire matrix
    /// first create data object of approporate size
    //transition_matrix[number_chromosomes].resize(recombination_rate.size()) ;
    transition_matrix[number_chromosomes].resize(transition_rates.size()) ;
    
    //// iterate across all positions and compute transition matrixes
    for ( int p = 0 ; p < transition_rates.size() ; p ++ ) {

        /// create actual transition matrix
        transition_matrix[number_chromosomes][p].set_size( transition_info.size(), transition_info.size() ) ;
        transition_matrix[number_chromosomes][p].fill( 0 ) ;
        
        /// population transitions by summing across all routes
        for ( int i = 0 ; i < transition_info.size() ; i ++ ) {
            for ( int j = 0 ; j < transition_info[i].size() ; j ++ ) {
                for ( std::map<vector<transition_information>,double>::iterator t = transition_info[i][j].begin() ; t != transition_info[i][j].end() ; ++ t ) {
                    double prob_t = 1 ;
                    for ( int r = 0 ; r < t->first.size() ; r ++ ) {
                        prob_t *= pow( transition_rates[p](t->first[r].start_state,t->first[r].end_state), t->first[r].transition_count ) ;
                    }
                    transition_matrix[number_chromosomes][p](j,i) += prob_t * t->second ;
                }
            }
        }        
    }
}

// calculates 2 vectors with transition rates going away from the site of interest
vector<vector<mat>> selection_transition_rates(selection point, vector<double> &recombination_rate, cmd_line &options) {
    vector<double> vecf ;
    vector<double> vecb ;
    //split_vector(point.pos, recombination_rate, vecb, vecf, options) ;

    double m = options.ancestry_pulses[1].proportion;
    int generations = options.ancestry_pulses[1].time ;
    int n = options.ne ; /// DOUBLE CHECK HAPLOID/DIPLOID!!    
    int tt = 0;

    // generates vector with allele frequencies of selected allele over time
    vector<double> sel_traject ;
    //double halfsel = 0.5 * point.sel; // test. remove
    selection_trajectory(sel_traject, point.sel, tt, m, generations, n) ; // change tt
    
    /*
    cerr << point << endl;
    for (int i = 0; i < sel_traject.size();i++) {
        cerr << sel_traject[i] << "\t";
    }*/
    //cerr << endl << "fwd_iter" << endl;

    //vector<mat> fwd_trans = fwd_iter(vecf, sel_traject, tt, m, generations, n) ;
    //vector<mat> back_trans = fwd_iter(vecb, sel_traject, tt, m, generations, n) ;

    // generates two vectors of transition rates, both going away from the site of interest in different directions

    //cerr << "fwd_vector" << endl;
    vector<mat> fwd_trans = fwd_iter(vecf, sel_traject, m, options.ne) ; //options.ne
    //cerr << endl << "back_vector" << endl;
    vector<mat> back_trans = fwd_iter(vecb, sel_traject, m, options.ne) ;

    vector<vector<mat>> tr_vector;
    tr_vector.push_back(fwd_trans);
    tr_vector.push_back(back_trans);
    return tr_vector;
}

// as above, but to use when printing expected genotype frequences across the chromosome
vector<vector<mat>> selection_transition_rates_genotypes(selection point, vector<double> &recombination_rate, cmd_line &options, vector<int> &position, vector < vector<double> > &genofreqs, vector <vector<double>> &split_vecs, map <double,vector<double>> &sel_trajectories) {
    point.sel = 0.5 * point.sel;

    cerr << "strg0: point " << point.pos << "  " << point.sel << endl;

    //vector<double> vecf ;
    //vector<double> vecb ;
    if (split_vecs.size() == 0) {
        split_vecs = split_vector(point.pos, recombination_rate, options) ;
    }

    cerr << "strg1: after vecf, recombination_rate.size()  " << recombination_rate.size() << endl;

    //vector<int> posvecf ;
    //vector<int> posvecb ;
    //split_vector_int(point.pos, position, posvecb, posvecf) ;

    cerr << "strg2: after posvecf  ";

    double m = options.ancestry_pulses[1].proportion;
    int generations = options.ancestry_pulses[1].time ;
    int n = options.ne ; /// DOUBLE CHECK HAPLOID/DIPLOID!!    
    int tt = 0;

    //cerr << "Stats. m=" << m << " generations=" << generations << " ne=" << n << endl;

    // generates vector with allele frequencies of selected allele over time
    vector<double> sel_traject ;
    map <double,vector<double>>::iterator it;
    it = sel_trajectories.find(point.sel);

    if (it == sel_trajectories.end()) {
        selection_trajectory(sel_traject, point.sel, tt, m, generations, n) ; // change tt
        sel_trajectories[point.sel] = sel_traject;
    }
    else {
        sel_traject = it->second;
    }
    
    cerr << "strg3: after selection_trajectory  " << endl;

    
    /* cerr << point << endl;
    for (int i = 0; i < sel_traject.size();i++) {
        cerr << sel_traject[i] << "\t";
    }*/
    //cerr << endl << "fwd_iter" << endl;

    //vector<mat> fwd_trans = fwd_iter(vecf, sel_traject, tt, m, generations, n) ;
    //vector<mat> back_trans = fwd_iter(vecb, sel_traject, tt, m, generations, n) ;

    // generates two vectors of transition rates, both going away from the site of interest in different directions

    vector<double> gf1;
    vector<double> gf2;
    genofreqs.push_back(gf1);
    genofreqs.push_back(gf2);


    cerr << "fwd_vector" << endl;
    vector<mat> fwd_trans = fwd_iter_genotype_freq(split_vecs[0], sel_traject, m, options.ne, genofreqs[0]) ; //options.ne
    //cerr << endl << "back_vector" << endl;
    vector<mat> back_trans = fwd_iter_genotype_freq(split_vecs[1], sel_traject, m, options.ne, genofreqs[1]) ;

    cerr << "strg4: genofreq  " << genofreqs.size() << "gf1 " << genofreqs[0].size() << endl;

    vector<vector<mat>> tr_vector;
    tr_vector.push_back(fwd_trans);
    tr_vector.push_back(back_trans);
    return tr_vector;
}

// function for calculating likelihood of parameters
double selection_evaluate_point(selection &point, vector<markov_chain> &markov_chain_information, map<int, vector<vector< map< vector<transition_information>, double > > > > &transition_matrix_information, vector<double> &recombination_rate, vector<int> &position, cmd_line &options, map<int,vector<vector<int> > > &state_changes ) {
    cerr << "BP2: Before transition rates." << endl;
    vector<vector<mat>> t_rates = selection_transition_rates(point, recombination_rate, options);
    //vector<vector<mat>> t_rates = selection_transition_rates_genotypes(point, recombination_rate, options, position); // test. remove
    
    cerr << "BP3: After transition rates." << endl;
    
    double comb_lnl = 0;
    bool go_backwards = false;
    //go_backwards = true;

    for (int i=0 ; i < 2 ; i++) {
        // transition matrix
        map<int,vector<mat> > transition_matrix ;
        for ( int m = 0 ; m < markov_chain_information.size() ; m ++ ) {
            selection_transition_matrix( transition_matrix, transition_matrix_information[markov_chain_information.at(m).number_chromosomes], recombination_rate, position, markov_chain_information.at(m).number_chromosomes, t_rates[i] ) ;
            // Delete maybe
            for ( int p = 0 ; p < markov_chain_information[m].ploidy_switch.size() ; p ++ ) {
                selection_transition_matrix( transition_matrix, transition_matrix_information[markov_chain_information[m].ploidy_switch[p]], recombination_rate, position, markov_chain_information[m].ploidy_switch[p], t_rates[i] ) ;
            }
        }
        cerr << "BP4: After transition matrix." << endl;
        /// compute transitions within a state
        vector<mat> interploidy_transitions ;
        //interploidy_transitions = create_interploidy_transitions( state_changes, vertex, options.ancestry_proportion ) ;
        
        /// now compute the forward probabilities
        double lnl = 0 ;
        cerr << "markov_chain_information.size()  " << markov_chain_information.size() << endl;
        for ( int m = 0 ; m < markov_chain_information.size() ; m ++ ) {
            //cerr << "Sample#: " << m << endl;
            lnl += markov_chain_information[m].selection_forward_probabilities( transition_matrix, interploidy_transitions, point, go_backwards ) ;
        }
        cerr << "BP5: After compute forward. " << i << " " << lnl << endl;
        comb_lnl += lnl;
        go_backwards = true;
    }
    point.lnl = comb_lnl;
    return comb_lnl ;
    // forward probabilities
    // other probabilities ??
} 

double selection_evaluate_point_genotypes(selection &point, vector<markov_chain> &markov_chain_information, map<int, vector<vector< map< vector<transition_information>, double > > > > &transition_matrix_information, vector<double> &recombination_rate, vector<int> &position, cmd_line &options, map<int,vector<vector<int> > > &state_changes, vector <vector<double>> &split_vecs, map <double,vector<double>> &sel_trajectories) {
    cerr << "BP2: Before transition rates." << endl;
    //vector<vector<mat>> t_rates = selection_transition_rates(point, recombination_rate, options);

    vector < vector<double> > genofreqs ;
    
    vector<vector<mat>> t_rates = selection_transition_rates_genotypes(point, recombination_rate, options, position, genofreqs, split_vecs, sel_trajectories); // test. remove
    
    /*for (int i = 0; i < t_rates[0].size(); i++) {
        cerr << t_rates[0][i] << endl;
    }*/

    cerr << "BP3: After transition rates." << endl;
    
    double comb_lnl = 0;
    bool go_backwards = false;
    //go_backwards = true;

    for (int i=0 ; i < 2 ; i++) {
        // transition matrix
        map<int,vector<mat> > transition_matrix ;
        for ( int m = 0 ; m < markov_chain_information.size() ; m ++ ) {
            selection_transition_matrix( transition_matrix, transition_matrix_information[markov_chain_information.at(m).number_chromosomes], recombination_rate, position, markov_chain_information.at(m).number_chromosomes, t_rates[i] ) ;
            // Delete maybe
            for ( int p = 0 ; p < markov_chain_information[m].ploidy_switch.size() ; p ++ ) {
                selection_transition_matrix( transition_matrix, transition_matrix_information[markov_chain_information[m].ploidy_switch[p]], recombination_rate, position, markov_chain_information[m].ploidy_switch[p], t_rates[i] ) ;
            }
        }
        cerr << "tr_matrix: " << transition_matrix.size() << endl;
        /// compute transitions within a state
        vector<mat> interploidy_transitions ;
        //interploidy_transitions = create_interploidy_transitions( state_changes, vertex, options.ancestry_proportion ) ;
        
        /// now compute the forward probabilities
        double lnl = 0 ;
        cerr << "markov_chain_information.size()  " << markov_chain_information.size() << endl;
        for ( int m = 0 ; m < markov_chain_information.size() ; m ++ ) {
        //for ( int m = 0 ; m < 1 ; m ++ ) {
            //cerr << "Sample#: " << m << endl;
            /*for (int j = 0; j < markov_chain_information[m].emission_probabilities.size();j++) {
                cerr << "markov_chain_information: " << markov_chain_information[m].emission_probabilities[j] << endl;
            }
            continue;*/
            lnl += markov_chain_information[m].selection_forward_probabilities_genotypes( transition_matrix, interploidy_transitions, point, go_backwards, genofreqs[i], position ) ;
        }
        cerr << "BP5: After compute forward. " << i << " " << lnl << endl;
        comb_lnl += lnl;
        go_backwards = true;
    }
    point.lnl = comb_lnl;
    return comb_lnl ;
    // forward probabilities
    // other probabilities ??
}


// function for determining if point is within bounds
// iterates until it is. (Somewhat dangerous. May cause slow-down or infinite loop if peak is outsite of bounds)
void selection_check_point(selection &point, cmd_line &options) {
    double pos_range_term = 0.1 ;
    double sel_range_term = 0.05 ;
    bool changed;
    int reps = 0;
    do {
        changed = false;
        if (point.pos > options.pos_max) {
            cout << "pos too large: " << point.pos << "\t";
            point.pos = point.pos - ((double) rand() / (RAND_MAX))*pos_range_term * ( options.pos_max - options.pos_min ) ;
            cout << point.pos << endl;
            changed = true;
        }
        else if (point.pos < options.pos_min)
        {
            cout << "pos too small: " << point.pos << "\t";
            point.pos = point.pos + ((double) rand() / (RAND_MAX))*pos_range_term * ( options.pos_max - options.pos_min ) ;
            cout << point.pos << endl;
            changed = true;
        }
        
        if (point.sel > options.sel_max) {
            cout << "sel too large: " << point.sel << "\t";
            point.sel = point.sel - ((double) rand() / (RAND_MAX))*sel_range_term * ( options.sel_max - options.sel_min ) ;
            cout << point.sel << endl;
            changed = true;
        }
        else if (point.sel < options.sel_min) {
            cout << "sel too small: " << point.sel << "\t";
            point.sel = point.sel + ((double) rand() / (RAND_MAX))*sel_range_term * ( options.sel_max - options.sel_min ) ;
            cout << point.sel << endl;
            changed = true;
        }
        if ( reps > 20) {
            cout << "selection_check_point run more than 20 times. Breaking." << endl;
            break;
        }
        reps++;
    } while (changed == true);
}

// function for reflecting, extending or contracting vertex. What it does depends on "mod" argument
selection selection_reflection(vector<selection> &vertex, double mod, vector<markov_chain> &markov_chain_information, map<int, vector<vector< map< vector<transition_information>, double > > > > &transition_matrix_information, vector<double> &recombination_rate, vector<int> &position, cmd_line &options, map<int,vector<vector<int> > > &state_changes ) {
    selection centroid;
    selection newpoint;

    // calculate centroid point
    centroid.sel = vertex[0].sel - (vertex[0].sel - vertex[1].sel)/2;
    centroid.pos = vertex[0].pos - (vertex[0].pos - vertex[1].pos)/2;

    // calculate new point to be used
    newpoint.sel = centroid.sel + mod*(centroid.sel - vertex[2].sel);
    newpoint.pos = centroid.pos + mod*(centroid.pos - vertex[2].pos);

    // checks if new point is within bounds
    selection_check_point(newpoint, options);

    // calculates likelihood
    selection_evaluate_point( newpoint, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes ) ;

    return newpoint;
}

// function for shinking vertex
vector<selection> selection_shrink(vector<selection> &vertex, double mod, vector<markov_chain> &markov_chain_information, map<int, vector<vector< map< vector<transition_information>, double > > > > &transition_matrix_information, vector<double> &recombination_rate, vector<int> &position, cmd_line &options, map<int,vector<vector<int> > > &state_changes ) {
    vector<selection> new_vertex;
    new_vertex.push_back(vertex[0]);
    
    selection point1;
    selection point2;

    point1.sel = vertex[0].sel - (vertex[0].sel - vertex[1].sel)/2;
    point1.pos = vertex[0].pos - (vertex[0].pos - vertex[1].pos)/2;

    point2.sel = vertex[0].sel - (vertex[0].sel - vertex[2].sel)/2;
    point2.pos = vertex[0].pos - (vertex[0].pos - vertex[2].pos)/2;
    
    selection_check_point(point1, options);
    selection_evaluate_point( point1, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes ) ;
    new_vertex.push_back(point1);

    selection_check_point(point2, options);
    selection_evaluate_point( point2, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes ) ;
    new_vertex.push_back(point2);

    return new_vertex;
}

// generates start vertex
vector<selection> selection_start_vertex(cmd_line &options) {
    vector<selection> vertex;

    // Chromosomal position of first point in vertex is within 0.2<x<0.8 of total sequence length.
    int pos_center_seq = (int)((options.pos_max+options.pos_min)*0.5);
    int pos_start_range = (int)((pos_center_seq-options.pos_min)*options.pos_limit); // NOTE: Maybe change 0.8 to option
    
    cout << "pos_center_seq: " << pos_center_seq << " pos_start_range: " << pos_start_range << " options.pos_min: " << options.pos_min << " options.pos_max: " << options.pos_max << " options.pos_limit: " << options.pos_limit << endl;

    double s_min_lim = options.sel_min + (options.sel_max - options.sel_min)*(1-options.sel_limit)*0.5;
    double s_max_lim = options.sel_min + (options.sel_max - options.sel_min)*(options.sel_limit)*0.5;

    selection point1;
    point1.pos = rand()%(2*pos_start_range + 1) + pos_center_seq-pos_start_range;
    point1.sel = double_rand(s_min_lim, s_max_lim);
    vertex.push_back(point1);

    // Chr. position for point 2 and 3 can be within 0.1 of seq.length compared to first point
    int vertex_range = (int)(pos_center_seq*0.2); // As above

    for (int i = 0; i < 2; i++) {
        selection point;
        point.pos = rand()%(2*vertex_range + 1) + point1.pos-vertex_range;
        point.sel = double_rand(s_min_lim, s_max_lim);
        vertex.push_back(point);
    }
    return vertex;
}

/*
vector<selection> old_selection_start_vertex(cmd_line &options) {
    vector<selection> vertex;

    for (int i; i < 3; i++) {
        selection point;
        point.pos = rand()%(options.pos_max-options.pos_min + 1) + options.pos_min;
        point.sel = double_rand(options.sel_min, options.sel_max);
        vertex.push_back(point);
    }
    return vertex;
}
*/


// function for using Nelder-Mead to find optimal values for selection and position
selection selection_nelder_mead(cmd_line &options, vector<markov_chain> &markov_chain_information, map<int, vector<vector< map< vector<transition_information>, double > > > > &transition_matrix_information, vector<double> &recombination_rate, vector<int> &position, map<int,vector<vector<int> > > &state_changes) {

    int r = 1;

    double alpha = 1 ; // for reflecting vertex
    double gamma = 2 ; // for extension vertex
    double rho = -0.5 ; // used for contracting vertex. Note negative value
    double sigma = 0.5 ; // for shrinking vertex

    double best_lnl = -1.7976931348623157E+308;
    selection best_optimum;
    vector<selection> vertex;

    // loop to repeat nelder mead search 20 times (should be changed)
    for (int reps=0 ; reps<20 ; reps++) {
        
        // generates random start vertex
        vertex = selection_start_vertex(options);
        best_optimum = vertex[0];

        // calculates likelihood for start vertex
        // Check parameters
        for ( int v = 0 ; v < vertex.size() ; v ++ ) {
                vertex[v].lnl = selection_evaluate_point( vertex[v], markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes ) ;
                cout << vertex[v] << endl;
            }
        
        // sorts start vertex
        selection_sort_vertex(vertex) ;
        
        int iteration = 0 ;

        // loop for Nelder-Mead search. Breaks when the change between each iteration is smaller than a tolerance
        while ( vertex[0].lnl-vertex.back().lnl > options.tolerance ) {
            /*if (vertex[0].pos == vertex[1].pos && vertex[0].pos == vertex[2].pos) {
                cout << "ABORTING. Degenerate position: " << vertex[0].pos << endl;
                break;
            }*/

            cout << "Vertex: " << r << "\t" << iteration << "\t" << vertex[0].lnl-vertex.back().lnl << "\t" ;
            iteration ++ ;

            cout << vertex[0] << "\t" << vertex[1] << "\t" << vertex[2] << "\t" << endl;

            // creates reflected vertex
            selection reflection = selection_reflection(vertex, alpha, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes);
            //cout << "R1" << endl;

            // checks new vertex and if an extension should be tried instead
            if (reflection < vertex[0] && vertex[1] < reflection) {
                vertex.back() = reflection ;
                cout << "Reflection1" << endl;
                selection_sort_vertex(vertex) ;
                continue ;
            }
            else if (vertex[0] < reflection) {
                selection extension = selection_reflection(vertex, gamma, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes);
                if ( reflection < extension ) {
                    vertex.back() = extension ;
                    cout << "Extension" << endl;
                }
                else {
                    vertex.back() = reflection ; 
                    cout << "Reflection2" << endl;
                }
                selection_sort_vertex(vertex) ;
                continue;
            }
        
            // if reflected or extended vertex is not good enough, tries contracting or shrinking vertex
            selection contraction = selection_reflection(vertex, rho, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes);
            
            if (vertex.back() < contraction) {
                vertex.back() = contraction ;
                cout << "Contraction" << endl;
            }
            else {
                vertex = selection_shrink(vertex, sigma, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes);
                cout << "Shrink" << endl;
            }
            selection_sort_vertex(vertex) ;
        }

        selection_sort_vertex(vertex) ;

        // outputs best point. This will be compared between each iteration of the N-M algorithm
        if ( best_lnl < vertex[0].lnl ) {
            best_optimum = vertex[0] ;
            best_lnl = vertex[0].lnl ;
        }
        cout << "Max point: " << vertex[0] <<  " best_lnl: " << best_lnl << endl;
    }
    
    cout << "END" << endl;
    cout << vertex[0] << "\t" << vertex[1] << "\t" << vertex[2] << "\t" << endl;
    return best_optimum;

}


// function for calculating likelihoods in a grid
// takes start, stop and step values for selection and position
void selection_grid(int p_start, int p_stop, int p_step, double s_start, double s_stop, double s_step, vector<markov_chain> &markov_chain_information, map<int, vector<vector< map< vector<transition_information>, double > > > > &transition_matrix_information, vector<double> &recombination_rate, vector<int> &position, cmd_line &options, map<int,vector<vector<int> > > &state_changes ) {

    map <double,vector<double>> sel_trajectories;

    for (int p = p_start; p < p_stop; p+=p_step) {
        
        vector <vector<double>> split_vecs;

        // generate neutral transition rate for normalization / calculating likelihood ratio
        selection point0;
        point0.pos = p;
        point0.sel = 0;
        selection_evaluate_point_genotypes( point0, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes, split_vecs, sel_trajectories ) ;
        //cout << "point0: " << point0.pos << "\t" << point0.sel << "\t" << setprecision(12) << point0.lnl << endl;

        for (double s = s_start; s < s_stop; s=s+s_step) {
            selection point;
            point.pos = p;
            point.sel = s;
            selection_evaluate_point_genotypes( point, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes, split_vecs, sel_trajectories ) ;
            cout << position[point.pos] << "\t" << point.sel << "\t" << setprecision(12) << point.lnl-point0.lnl << endl;
        }
    }
}

selection gs_search() {}

void selection_golden_section(vector<markov_chain> &markov_chain_information, map<int, vector<vector< map< vector<transition_information>, double > > > > &transition_matrix_information, vector<double> &recombination_rate, vector<int> &position, cmd_line &options, map<int,vector<vector<int> > > &state_changes) {
    map <double,vector<double>> sel_trajectories;
    double GR = (sqrt(5) + 1) / 2;

    for (int p = options.gs_pstart; p < options.gs_pstop; p+=options.gs_pstep) {
        vector <vector<double>> split_vecs;

        selection point0;
        selection point1;
        selection point2;
        selection point3;

        point0.pos = p;
        point1.pos = p;
        point2.pos = p;
        point3.pos = p;

        point0.sel = options.gs_sstart;
        point1.sel = options.gs_sstop;
        point2.sel = point1.sel - (point1.sel - point0.sel) / GR;
        point3.sel = point0.sel + (point1.sel - point0.sel) / GR;

        selection_evaluate_point_genotypes( point0, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes, split_vecs, sel_trajectories ) ;
        selection_evaluate_point_genotypes( point1, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes, split_vecs, sel_trajectories ) ;
        selection_evaluate_point_genotypes( point2, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes, split_vecs, sel_trajectories ) ;
        selection_evaluate_point_genotypes( point3, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes, split_vecs, sel_trajectories ) ;

        int i = 0;

        while (abs(point2.sel - point3.sel) > options.gs_precision || i < options.gs_max_iterations) {
            if (point2.lnl > point3.lnl) {
                point1 = point3;
                point3 = point2;
                point2.sel = point1.sel - (point1.sel - point0.sel) / GR;
                selection_evaluate_point_genotypes( point2, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes, split_vecs, sel_trajectories ) ;
            }
            else {
                point0 = point2;
                point2 = point3;
                point3.sel = point0.sel + (point1.sel - point0.sel) / GR;
                selection_evaluate_point_genotypes( point3, markov_chain_information, transition_matrix_information, recombination_rate, position, options, state_changes, split_vecs, sel_trajectories ) ;
            }
            i++;
        }

        cout << position[point.pos] << "\t" << (point2.sel-point3.sel)/2 << "\t" << setprecision(12) << (point2.lnl-point3.lnl)/2 << "\t" << i << endl;
    }
}




#endif
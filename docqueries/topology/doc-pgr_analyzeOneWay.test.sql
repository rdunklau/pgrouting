/* --q1 */
	SELECT pgr_analyzeOneWay('edge_table',
        ARRAY['', 'B', 'TF'],
        ARRAY['', 'B', 'FT'],
        ARRAY['', 'B', 'FT'],
        ARRAY['', 'B', 'TF'],
        oneway:='dir');
/* --q1.1 */

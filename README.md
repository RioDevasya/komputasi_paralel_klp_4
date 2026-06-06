# komputasi paralel kelompok 4
<pre>
Ramadhani Pasha Islami  2430305030009
Berliantony Delon Usup  2430305030023
Rio Devasya             2430205030020
Adryan Dwislamy	        2430305030029
</pre>

## License
<pre>
This project is licensed under the GNU General Public License v3.0 (GPL-3.0).

You are free to use, modify, and redistribute this software under the terms of the GPL-3.0 license. 
Any distributed modifications or derivative works must also be released under GPL-compatible terms.

This project uses OpenMP and MPI libraries for parallel and distributed computation. 
OpenMP and MPI implementations are distributed under their own licenses and remain subject to their respective terms.

See the LICENSE file for details.
</pre>
<pre>
sebelum dijalankan, tekan CTRL+SHIFT+B untuk build di VSCode [Build All: OpenMP, OpenMPI]. 
Jika manual, lihat command di task.json. 
Setelah build, cara menjalankan (Linux only):

./main_parallel > output.txt 2>&1 
mpirun -np 1 main_distributed >> output.txt 2>&1
mpirun -np 2 main_distributed >> output.txt 2>&1 
mpirun -np 4 main_distributed >> output.txt 2>&1 
mpirun -np 1 distributed_and_parallel >> output.txt 2>&1 
mpirun -np 2 distributed_and_parallel >> output.txt 2>&1
mpirun -np 4 distributed_and_parallel >> output.txt 2>&1

Untuk testing:
./test_sorting
</pre>

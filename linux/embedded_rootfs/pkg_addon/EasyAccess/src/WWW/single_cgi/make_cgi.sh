if [ -f ./single_cgi_files.c ]; then
    unlink single_cgi_files.c
fi
for i in `cat CGI_LIST`
do
        if [ -f `pwd`/../cgi-bin/$i ]
        then
                echo "" >> single_cgi_files.c
                echo "/* ---------------------$i------------------------- */" >> single_cgi_files.c
                cat `pwd`/../cgi-bin/$i >> single_cgi_files.c
        fi
done

if [ "$CONFIG_EPS" == "y" ];then
for i in `cat EPSCGI_LIST`
do
        if [ -f `pwd`/../cgi-bin/$i ]
        then
                echo "/* ---------------------$i------------------------- */" >> single_cgi_files.c
                cat `pwd`/../cgi-bin/$i >> single_cgi_files.c
        fi
done
fi

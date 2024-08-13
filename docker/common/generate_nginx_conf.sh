#!/bin/sh

NGINX_CONF="/etc/nginx/conf.d/generated_routes.conf"

rm -f $NGINX_CONF

touch $NGINX_CONF

while IFS= read -r line || [ -n "$line" ]
do
    route=$(echo $line | awk '{print $1}')
    action=$(echo $line | awk '{print $2}')

    echo "location $route {" >> $NGINX_CONF
    if [ "$action" = "allow" ]; then
        echo "    allow all;" >> $NGINX_CONF
    else
        echo "    deny all;" >> $NGINX_CONF
    fi
    echo "}" >> $NGINX_CONF
done < /app/routes.txt

docker build -t nitro_dev .

echo 'Successfully built the base image, wil; build the dev image!🎉🎉🎉🎉' 

# rebuild the dev image because sometimes the vim config might be changed
docker build --no-cache -f ./dev.Dockerfile -t nitro_dev_env .

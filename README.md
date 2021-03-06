# Description #
Nowadays most search engines provide some support for image retrieval, which allows you, for example, to search the web for images that are similar to a given image. If a search engine used a single process to search a large number of images to find the most similar one it would take a very long time to get the results. 
# Process #
This project allows users to input an image file and a directory (think of this as the global dataset) to search for the most similar image. The user can then choose to either perform the search using a single process or to use a parallel version that forks new processes and manages communication through various pipes. In reality, determining how similar two images are is a complex problem, which is in fact an active area of research. The project uses a simple metric based on the Euclidean distance between two pixels of different images. The purpose of this project was to demonstrate drastic performance differences between delegating a task to a single process (one_process.c) vs a parallel process (image_retrieval.c).
# Workflow #
![](images/ris-a-1.png)

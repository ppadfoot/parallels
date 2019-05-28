#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#define ALIVE 'X'
#define DEAD '.'

typedef struct {
    int n;
    int iterations;
    bool **field;
} args;

bool *init_row(int n) {
    return (bool *) malloc(n * sizeof(bool));
}

bool **init_chunk(int chunk_size, int n) {
    bool **chunk = (bool **) malloc(chunk_size * sizeof(bool *));
    for (int i = 0; i < chunk_size; i++)
        chunk[i] = init_row(n);
    return chunk;
}

void serialize_chunk(bool **chunk, int height, int width, bool *result) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            result[i * width + j] = chunk[i][j];
}

void deserialize_chunk(bool *chunk, int height, int width, bool **result) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            result[i][j] = chunk[i * width + j];
}

void free_chunk(bool **chunk, int size) {
    for (int i = 0; i < size; i++)
        free(chunk[i]);
    free(chunk);
}

bool parse_args(int argc, char *argv[], int n_threads, args *args_) {
    if (argc != 5) {
        printf("Usage: %s N input_file iterations output_file\n", argv[0]);
        return false;
    }

    int n = atoi(argv[1]);
    if (n % n_threads != 0) {
        printf("N mod n_threads != 0 for N = %d, n_threads = %d\n", n, n_threads);
        return false;
    }

    args_->n = n;
    args_->iterations = atoi(argv[3]);

    FILE *input = fopen(argv[2], "r");
    if (!input) {
        printf("Unable to open file: %s\n", argv[2]);
        return false;
    }

    char *line = (char *) malloc((n + 1) * sizeof(char));
    args_->field = init_chunk(n, n);
    for (int i = 0; i < n; i++) {
        fscanf(input, "%s", line);
        for (int j = 0; j < n; j++)
            args_->field[i][j] = line[j] == ALIVE ? true : false;
    }
    fclose(input);
    return true;
}

void recalculate(bool **chunk, int chunk_size, int n, const bool *from_up, const bool *from_down) {
    bool **new_chunk = init_chunk(chunk_size, n);
    for (int i = 0; i < chunk_size; i++)
        for (int j = 0; j < n; j++) {
            int sum = 0;
            if (i > 0 && j > 0)
                sum += chunk[i - 1][j - 1];
            if (i > 0)
                sum += chunk[i - 1][j];
            if (j > 0)
                sum += chunk[i][j - 1];

            if (i < chunk_size - 1 && j < n - 1)
                sum += chunk[i + 1][j + 1];
            if (i < chunk_size - 1)
                sum += chunk[i + 1][j];
            if (j < n - 1)
                sum += chunk[i][j + 1];

            if (i > 0 && j < n - 1)
                sum += chunk[i - 1][j + 1];
            if (i < chunk_size - 1 && j > 0)
                sum += chunk[i + 1][j - 1];

            if (j == 0)
                sum += from_up[i];
            if (j == 0 && i > 0)
                sum += from_up[i - 1];
            if (j == 0 && i < chunk_size - 1)
                sum += from_up[i + 1];

            if (j == n - 1)
                sum += from_down[i];
            if (j == n - 1 && i > 0)
                sum += from_down[i - 1];
            if (j == n - 1 && i < chunk_size - 1)
                sum += from_down[i + 1];

            new_chunk[i][j] = chunk[i][j] && sum == 2 || sum == 3;
        }


    for (int i = 0; i < chunk_size; i++)
        for (int j = 0; j < n; j++)
            chunk[i][j] = new_chunk[i][j];

    free_chunk(new_chunk, chunk_size);
}

int main(int argc, char *argv[]) {
    double start, end;
    int rank, size, tag = 1;
    int n, iterations, chunk_size;
    bool **chunk = NULL;
    bool *chunk_serialized = NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    if (rank == 0) {
        args parsed;
        if (!parse_args(argc, argv, size, &parsed)) {
            MPI_Finalize();
            return -1;
        }

        n = parsed.n;
        iterations = parsed.iterations;
        chunk_size = n / size;

        int info[] = {n, iterations, chunk_size};
        for (int i = 1; i < size; i++)
            MPI_Send(info, 3, MPI_INT, i, tag, MPI_COMM_WORLD);

        chunk = init_chunk(chunk_size, n);
        chunk_serialized = init_row(chunk_size * n);
        for (int z = 1; z < size; z++) {
            for (int i = 0; i < chunk_size; i++)
                for (int j = 0; j < n; j++)
                    chunk[i][j] = parsed.field[i + (z * chunk_size)][j];
            serialize_chunk(chunk, chunk_size, n, chunk_serialized);
            MPI_Send(chunk_serialized, n * chunk_size, MPI_C_BOOL, z, tag, MPI_COMM_WORLD);
        }

        for (int i = 0; i < chunk_size; i++)
            for (int j = 0; j < n; j++)
                chunk[i][j] = parsed.field[i][j];

        free_chunk(parsed.field, n);
    } else {
        int info[3];
        MPI_Status status;
        MPI_Recv(info, 3, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

        n = info[0];
        iterations = info[1];
        chunk_size = info[2];

        chunk = init_chunk(chunk_size, n);
        chunk_serialized = init_row(chunk_size * n);
        MPI_Recv(chunk_serialized, n * chunk_size, MPI_C_BOOL, 0, tag, MPI_COMM_WORLD, &status);
        deserialize_chunk(chunk_serialized, chunk_size, n, chunk);
    }

    bool *to_down = init_row(n);
    bool *to_up = init_row(n);
    bool *from_down = init_row(n);
    bool *from_up = init_row(n);
    MPI_Status status;
    for (int _ = 0; _ < iterations; _++) {
        if (rank != size - 1) {
            for (int j = 0; j < n; j++)
                to_down[j] = chunk[chunk_size - 1][j];
            MPI_Send(to_down, n, MPI_C_BOOL, rank + 1, tag, MPI_COMM_WORLD);
        } else {
            for (int k = 0; k < n; k++) from_down[k] = false;
        }

        if (rank != 0) {
            MPI_Recv(from_up, n, MPI_C_BOOL, rank - 1, tag, MPI_COMM_WORLD, &status);
        } else {
            for (int k = 0; k < n; k++)
                from_up[k] = false;
        }

        if (rank != 0) {
            for (int j = 0; j < n; j++)
                to_up[j] = chunk[0][j];
            MPI_Send(to_up, n, MPI_C_BOOL, rank - 1, tag, MPI_COMM_WORLD);
        }

        if (rank != size - 1) {
            MPI_Recv(from_down, n, MPI_C_BOOL, rank + 1, tag, MPI_COMM_WORLD, &status);
        }

        recalculate(chunk, chunk_size, n, from_up, from_down);
    }

    if (rank == 0) {
        FILE *output = fopen(argv[4], "w");
        for (int i = 0; i < chunk_size; i++) {
            for (int j = 0; j < n; j++)
                fprintf(output, "%c", chunk[i][j] ? ALIVE : DEAD);
            fprintf(output, "\n");
        }
        for (int z = 1; z < size; z++) {
            MPI_Recv(chunk_serialized, n * chunk_size, MPI_C_BOOL, z, tag, MPI_COMM_WORLD, &status);
            deserialize_chunk(chunk_serialized, chunk_size, n, chunk);
            for (int i = 0; i < chunk_size; i++) {
                for (int j = 0; j < n; j++)
                    fprintf(output, "%c", chunk[i][j] ? ALIVE : DEAD);
                fprintf(output, "\n");
            }
        }

        fclose(output);
    } else {
        serialize_chunk(chunk, chunk_size, n, chunk_serialized);
        MPI_Send(chunk_serialized, n * chunk_size, MPI_C_BOOL, 0, tag, MPI_COMM_WORLD);
    }

    free_chunk(chunk, chunk_size);
    free(chunk_serialized);
    free(to_down);
    free(to_up);
    free(from_down);
    free(from_up);

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();

    MPI_Finalize();

    printf("n_threads=%d, Runtime=%f\n", size, end - start);
}

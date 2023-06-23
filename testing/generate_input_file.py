import argparse
import random


def generate_integers(num_integers):
    return [random.randint(0, 65535) for _ in range(num_integers)]


def write_integers_to_file(integers, filename):
    with open(filename, 'w') as file:
        file.write('\n'.join(map(str, integers)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Generate and write 16-bit integers to a file')
    parser.add_argument('count', type=int,
                        help='Number of integers to generate')
    parser.add_argument('filename', help='Output filename')

    args = parser.parse_args()

    integers = generate_integers(args.count)
    write_integers_to_file(integers, args.filename)
    print(f'{args.count} integers written to {args.filename} successfully.')

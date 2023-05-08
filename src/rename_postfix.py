import os
import argparse

def parse_args():
  parser = argparse.ArgumentParser()
  parser.add_argument(
    'origin_postfix', type=str, default='.cpp'
  )
  parser.add_argument(
    'new_postfix', type=str, default='.cc'
  )
  parser.add_argument(
    '-d', '--dir', help="Rename files under specified directory", type=str, default='.'
  )
  parser.add_argument(
    '-r', action="store_true", help="Recurrently rename all files under the root directory"
  )
  parser.add_argument(
    '-n', action="store_true", help="If -n is specified, print the commands but not execute them"
  )

  return parser.parse_args()

if __name__ == '__main__':
  options = parse_args()

  root_folder = options.dir
  os.chdir(root_folder)
  file_list = os.listdir(root_folder)

  def rename_files(f_list) -> list:
    sub_directories = []
    for f in f_list:
      if (os.path.isdir(f)):
        sub_directories.append(f)

      if (f.endswith(options.origin_postfix)):
        new_f = f.replace(options.origin_postfix, options.new_postfix)
        if (not options.n):
          os.rename(f, new_f)
        print('Renamed {} as {}'.format(f, new_f))
    return sub_directories

  if (not options.r):
    rename_files(file_list)
  else:
    sub_dir = rename_files(file_list)
    while (len(sub_dir) != 0):
      temp_list = []
      for sub in sub_dir:
        os.chdir(sub)
        sub_sub_dir = rename_files(os.listdir(os.getcwd()))
        if (len(sub_sub_dir) != 0):
          temp_list.append(sub_sub_dir)
        os.chdir('..')
      sub_dir = temp_list
  
  print("Done.")
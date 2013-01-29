using System;
using System.Collections;
using System.Collections.Generic;

namespace TyphoonAdapter.DCC
{
    public class DCCCommandCollection : ICollection<DCCCommand>
    {
        #region Fields
        private readonly List<DCCCommand> list = new List<DCCCommand>();
        #endregion

        #region Properties
        public int Count
        {
            get { return list.Count; }
        }
        public bool IsReadOnly
        {
            get { return false; }
        }
        public virtual DCCCommand this[int index]
        {
            get
            {
                if (index < 0 || index > list.Count - 1)
                    throw new ArgumentOutOfRangeException("index");
                return list[index];
            }
            set
            {
                if (index < 0 || index > list.Count - 1)
                    throw new ArgumentOutOfRangeException("index");
                list[index] = value;
            }
        }
        #endregion

        #region Public methods
        public IEnumerator<DCCCommand> GetEnumerator()
        {
            return list.GetEnumerator();
        }
        IEnumerator IEnumerable.GetEnumerator()
        {
            return list.GetEnumerator();
        }

        public virtual void Add(DCCCommand item)
        {
            list.Add(item);
        }
        public virtual void Insert(int index, DCCCommand item)
        {
            list.Insert(index, item);
        }
        public bool Remove(DCCCommand item)
        {
            return list.Remove(item);
        }
        public void RemoveAt(int idx)
        {
            list.RemoveAt(idx);
        }
        public void Clear()
        {
            list.Clear();
        }

        public void CopyTo(DCCCommand[] array, int arrayIndex)
        {
            list.CopyTo(array, arrayIndex);
        }
        public bool Contains(DCCCommand item)
        {
            return list.Contains(item);
        }

        public void MoveUp(DCCCommand item)
        {
            if (list.Contains(item))
            {
                int idx = list.IndexOf(item);
                if (idx == 0)
                    idx = list.Count - 1;
                else
                    idx--;
                list.Remove(item);
                list.Insert(idx, item);
            }
        }
        public void MoveDown(DCCCommand item)
        {
            if (list.Contains(item))
            {
                int idx = list.IndexOf(item);
                if (idx == list.Count - 1)
                    idx = 0;
                else
                    idx++;
                list.Remove(item);
                list.Insert(idx, item);
            }
        }
        #endregion
    }
}

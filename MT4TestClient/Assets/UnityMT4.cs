using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO.Pipes;

public class UnityMT4 : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
//        var ps = new pipese
new NamedPipeClientStream(".","UnityMT4_FromUnity", PipeDirection.Out, PipeOptions.None,  )
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
